//
// Copyright 2016-2019 Timo Kloss
//
// This file is part of LowRes NX.
//
// LowRes NX is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes NX is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes NX.  If not, see <http://www.gnu.org/licenses/>.
//

#include "interpreter.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include "core.h"
#include "default_characters.h"
#include "cmd_audio.h"
#include "cmd_control.h"
#include "cmd_variables.h"
#include "cmd_data.h"
#include "cmd_strings.h"
#include "cmd_memory.h"
#include "cmd_text.h"
#include "cmd_maths.h"
#include "cmd_background.h"
#include "cmd_screen.h"
#include "cmd_sprites.h"
#include "cmd_io.h"
#include "cmd_files.h"
#include "cmd_subs.h"
#include "string_utils.h"

struct TypedValue itp_evaluateExpressionLevel(struct Core *core, int level);
struct TypedValue itp_evaluatePrimaryExpression(struct Core *core);
struct TypedValue itp_evaluateFunction(struct Core *core);
enum ErrorCode itp_evaluateCommand(struct Core *core);

void itp_init(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    interpreter->romDataManager.data = core->machine->cartridgeRom;
    
    // global null string
    interpreter->nullString = rcstring_new(NULL, 0);
    if (!interpreter->nullString) exit(EXIT_FAILURE);
}

void itp_deinit(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    itp_freeProgram(core);
    
    // Free null string
    if (interpreter->nullString)
    {
        rcstring_release(interpreter->nullString);
        interpreter->nullString = NULL;
    }
}

struct CoreError itp_compileProgram(struct Core *core, const char *sourceCode)
{
    struct Interpreter *interpreter = core->interpreter;
    
    itp_freeProgram(core);
    
    // Parse source code
    
    interpreter->sourceCode = uppercaseString(sourceCode);
    if (!interpreter->sourceCode) return err_makeCoreError(ErrorOutOfMemory, -1);
    
    struct CoreError error = tok_tokenizeUppercaseProgram(&interpreter->tokenizer, interpreter->sourceCode);
    if (error.code != ErrorNone)
    {
        return error;
    }
    
    struct DataManager *romDataManager = &interpreter->romDataManager;
    error = data_uppercaseImport(romDataManager, interpreter->sourceCode, false);
    if (error.code != ErrorNone) return error;

    // add default characters if ROM entry 0 is unused
    struct DataEntry *entry0 = &romDataManager->entries[0];
    if (entry0->length == 0 && (DATA_SIZE - data_currentSize(romDataManager)) >= 1024)
    {
        data_setEntry(romDataManager, 0, "FONT", (uint8_t *)DefaultCharacters, 1024);
    }
    
    // Prepare commands
    
    interpreter->pc = interpreter->tokenizer.tokens;
    interpreter->pass = PassPrepare;
    interpreter->exitEvaluation = false;
    interpreter->subLevel = 0;
    interpreter->numLabelStackItems = 0;
    interpreter->isSingleLineIf = false;
    
    enum ErrorCode errorCode;
    do
    {
        errorCode = itp_evaluateCommand(core);
    }
    while (errorCode == ErrorNone && interpreter->pc->type != TokenUndefined);
    
    if (errorCode != ErrorNone) return err_makeCoreError(errorCode, interpreter->pc->sourcePosition);
    
    if (interpreter->numLabelStackItems > 0)
    {
        struct LabelStackItem *item = &interpreter->labelStackItems[interpreter->numLabelStackItems - 1];
        switch (item->type)
        {
            case LabelTypeIF:
            case LabelTypeELSEIF:
            case LabelTypeELSE:
                errorCode = ErrorIfWithoutEndIf;
                break;
                
            case LabelTypeFOR:
                errorCode =  ErrorForWithoutNext;
                break;
                
            case LabelTypeDO:
                errorCode =  ErrorDoWithoutLoop;
                break;
                
            case LabelTypeREPEAT:
                errorCode =  ErrorRepeatWithoutUntil;
                break;
                
            case LabelTypeWHILE:
                errorCode =  ErrorWhileWithoutWend;
                break;
                
            case LabelTypeSUB:
                errorCode = ErrorSubWithoutEndSub;
                break;
                
            case LabelTypeFORVar:
            case LabelTypeFORLimit:
            case LabelTypeGOSUB:
            case LabelTypeCALL:
            case LabelTypeONCALL:
                // should not happen in compile time
                errorCode = ErrorSyntax;
                break;
        }
        if (errorCode != ErrorNone)
        {
            return err_makeCoreError(errorCode, item->token->sourcePosition);
        }
    }
    
    // prepare for run
    
    interpreter->pc = interpreter->tokenizer.tokens;
    interpreter->cycles = 0;
    interpreter->interruptOverCycles = 0;
    interpreter->pass = PassRun;
    interpreter->state = StateEvaluate;
    interpreter->mode = ModeNone;
    interpreter->handlesPause = true;
    interpreter->currentDataToken = interpreter->firstData;
    interpreter->currentDataValueToken = interpreter->firstData ? interpreter->firstData + 1 : NULL;
    interpreter->isSingleLineIf = false;
    interpreter->lastFrameIOStatus.value = 0;
    interpreter->seed = 0;
    interpreter->isKeyboardOptional = false;
    
    memset(&interpreter->textLib, 0, sizeof(struct TextLib));
    memset(&interpreter->spritesLib, 0, sizeof(struct SpritesLib));
    memset(&interpreter->audioLib, 0, sizeof(struct AudioLib));
    interpreter->textLib.core = core;
    interpreter->spritesLib.core = core;
    interpreter->audioLib.core = core;

    return err_noCoreError();
}

void itp_runProgram(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    switch (interpreter->state)
    {
        case StateEvaluate:
        {
            if (interpreter->waitCount > 0)
            {
                --interpreter->waitCount;
                break;
            }
            
            interpreter->mode = ModeMain;
            interpreter->exitEvaluation = false;
            enum ErrorCode errorCode = ErrorNone;
            
            while (   errorCode == ErrorNone
                   && interpreter->cycles < MAX_CYCLES_TOTAL_PER_FRAME
                   && interpreter->state == StateEvaluate
                   && !interpreter->exitEvaluation)
            {
                errorCode = itp_evaluateCommand(core);
            }
            
            if (interpreter->cycles >= MAX_CYCLES_TOTAL_PER_FRAME)
            {
                machine_suspendEnergySaving(core, 2);
            }
            
            interpreter->mode = ModeNone;
            if (errorCode != ErrorNone)
            {
                itp_endProgram(core);
                delegate_interpreterDidFail(core, err_makeCoreError(errorCode, interpreter->pc->sourcePosition));
            }
            break;
        }
            
        case StateInput:
        {
            if (txtlib_inputUpdate(&interpreter->textLib))
            {
                interpreter->state = StateEvaluate;
                cmd_endINPUT(core);
            }
            break;
        }
            
        case StateNoProgram:
        case StatePaused:
        case StateEnd:
        case StateWaitForDisk:
            break;
    }
}

void itp_runInterrupt(struct Core *core, enum InterruptType type)
{
    struct Interpreter *interpreter = core->interpreter;
    
    switch (interpreter->state)
    {
        case StateEvaluate:
        case StateInput:
        case StatePaused:
        case StateWaitForDisk:
        {
            struct Token *startToken = NULL;
            int maxCycles;
            
            int mainCycles = interpreter->cycles;
            interpreter->cycles = 0;
            
            switch (type)
            {
                case InterruptTypeRaster:
                    startToken = interpreter->currentOnRasterToken;
                    maxCycles = MAX_CYCLES_PER_RASTER;
                    break;
                    
                case InterruptTypeVBL:
                    startToken = interpreter->currentOnVBLToken;
                    maxCycles = MAX_CYCLES_PER_VBL;
                    // update audio player
                    audlib_update(&interpreter->audioLib);
                    break;
            }
            
            if (startToken)
            {
                interpreter->mode = ModeInterrupt;
                interpreter->exitEvaluation = false;
                struct Token *pc = interpreter->pc;
                interpreter->pc = startToken;
                interpreter->subLevel++;
                
                enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeONCALL, NULL);
                
                while (   errorCode == ErrorNone
                       // cycles can exceed interrupt limit (see interruptOverCycles), but there is still a hard limit for extreme cases
                       && interpreter->cycles < MAX_CYCLES_TOTAL_PER_FRAME
                       && !interpreter->exitEvaluation)
                {
                    errorCode = itp_evaluateCommand(core);
                }
                
                interpreter->mode = ModeNone;
                
                if (interpreter->cycles >= MAX_CYCLES_TOTAL_PER_FRAME)
                {
                    itp_endProgram(core);
                    delegate_interpreterDidFail(core, err_makeCoreError(ErrorTooManyCPUCyclesInInterrupt, interpreter->pc->sourcePosition));
                }
                else if (errorCode != ErrorNone)
                {
                    itp_endProgram(core);
                    delegate_interpreterDidFail(core, err_makeCoreError(errorCode, interpreter->pc->sourcePosition));
                }
                else
                {
                    interpreter->pc = pc;
                }
            }
            
            // calculate cycles exceeding limit
            interpreter->interruptOverCycles += interpreter->cycles - maxCycles;
            if (interpreter->interruptOverCycles < 0)
            {
                interpreter->interruptOverCycles = 0;
            }
            
            // sum of interrupt's and main cycle count
            interpreter->cycles += mainCycles;
            
            break;
        }
            
        case StateNoProgram:
        case StateEnd:
            break;
    }

}

void itp_didFinishVBL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // remember this frame's IO
    for (int i = 0; i < NUM_GAMEPADS; i++)
    {
        interpreter->lastFrameGamepads[i] = core->machine->ioRegisters.gamepads[i];
    }
    interpreter->lastFrameIOStatus = core->machine->ioRegisters.status;
    
    // timer
    interpreter->timer++;
    if (interpreter->timer >= TIMER_WRAP_VALUE)
    {
        interpreter->timer = 0;
    }
    
    // pause
    if (core->machine->ioRegisters.status.pause)
    {
        if (interpreter->handlesPause && interpreter->state == StateEvaluate)
        {
            interpreter->state = StatePaused;
            overlay_updateState(core);
            core->machine->ioRegisters.status.pause = 0;
        }
        else if (interpreter->state == StatePaused)
        {
            interpreter->state = StateEvaluate;
            overlay_updateState(core);
            core->machine->ioRegisters.status.pause = 0;
        }
    }
    
    // CPU load (rounded up)
    int currentCpuLoad = (interpreter->cycles * 100 + MAX_CYCLES_TOTAL_PER_FRAME - 1) / MAX_CYCLES_TOTAL_PER_FRAME;
    if (currentCpuLoad > interpreter->cpuLoadMax)
    {
        interpreter->cpuLoadMax = currentCpuLoad;
    }
    ++interpreter->cpuLoadTimer;
    if (interpreter->cpuLoadTimer >= 30)
    {
        interpreter->cpuLoadTimer = 0;
        interpreter->cpuLoadDisplay = interpreter->cpuLoadMax;
        interpreter->cpuLoadMax = currentCpuLoad;
    }
    
    // reset CPU cycles
    interpreter->cycles = interpreter->cycles - MAX_CYCLES_TOTAL_PER_FRAME;
    if (interpreter->cycles < 0)
    {
        interpreter->cycles = 0;
    }
}

void itp_endProgram(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    interpreter->state = StateEnd;
    interpreter->interruptOverCycles = 0;
}

void itp_freeProgram(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    interpreter->state = StateNoProgram;
    interpreter->firstData = NULL;
    interpreter->lastData = NULL;
    interpreter->currentDataToken = NULL;
    interpreter->currentDataValueToken = NULL;
    interpreter->currentOnRasterToken = NULL;
    interpreter->currentOnVBLToken = NULL;
    interpreter->lastVariableValue = NULL;
    
    var_freeSimpleVariables(interpreter, SUB_LEVEL_GLOBAL);
    var_freeArrayVariables(interpreter, SUB_LEVEL_GLOBAL);
    tok_freeTokens(&interpreter->tokenizer);
    
    if (interpreter->sourceCode)
    {
        free((void *)interpreter->sourceCode);
        interpreter->sourceCode = NULL;
    }
}

enum ValueType itp_getIdentifierTokenValueType(struct Token *token)
{
    if (token->type == TokenIdentifier)
    {
        return ValueTypeFloat;
    }
    else if (token->type == TokenStringIdentifier)
    {
        return ValueTypeString;
    }
    return ValueTypeNull;
}

union Value *itp_readVariable(struct Core *core, enum ValueType *type, enum ErrorCode *errorCode, bool forWriting)
{
    struct Interpreter *interpreter = core->interpreter;
    
    struct Token *tokenIdentifier = interpreter->pc;
    
    if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
    {
        *errorCode = ErrorSyntax;
        return NULL;
    }
    
    enum ValueType varType = itp_getIdentifierTokenValueType(tokenIdentifier);
    if (type)
    {
        *type = varType;
    }
    
    int symbolIndex = tokenIdentifier->symbolIndex;
    ++interpreter->pc;
    ++interpreter->cycles;
    
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // array
        ++interpreter->pc;
        
        struct ArrayVariable *variable = NULL;
        if (interpreter->pass == PassRun)
        {
            variable = var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel);
            if (!variable)
            {
                *errorCode = ErrorArrayNotDimensionized;
                return NULL;
            }
        }
        
        int indices[MAX_ARRAY_DIMENSIONS];
        int numDimensions = 0;
        
        for (int i = 0; i < MAX_ARRAY_DIMENSIONS; i++)
        {
            struct TypedValue indexValue = itp_evaluateExpression(core, TypeClassNumeric);
            if (indexValue.type == ValueTypeError)
            {
                *errorCode = indexValue.v.errorCode;
                return NULL;
            }
            
            numDimensions++;
            
            if (interpreter->pass == PassRun)
            {
                if (numDimensions <= variable->numDimensions && (indexValue.v.floatValue < 0 || indexValue.v.floatValue >= variable->dimensionSizes[i]))
                {
                    *errorCode = ErrorIndexOutOfBounds;
                    return NULL;
                }
                
                indices[i] = indexValue.v.floatValue;
            }
            
            if (interpreter->pc->type == TokenComma)
            {
                ++interpreter->pc;
            }
            else
            {
                break;
            }
        }
        
        if (interpreter->pc->type != TokenBracketClose)
        {
            *errorCode = ErrorSyntax;
            return NULL;
        }
        ++interpreter->pc;
        
        if (interpreter->pass == PassRun)
        {
            if (numDimensions != variable->numDimensions)
            {
                *errorCode = ErrorWrongNumberOfDimensions;
                return NULL;
            }
            return var_getArrayValue(interpreter, variable, indices);
        }
    }
    else
    {
        // simple variable
        if (interpreter->pass == PassRun)
        {
            struct SimpleVariable *variable = var_getSimpleVariable(interpreter, symbolIndex, interpreter->subLevel);
            if (!variable)
            {
                // check if variable name is already used for array
                if (var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel))
                {
                    *errorCode = ErrorArrayVariableWithoutIndex;
                    return NULL;
                }
                if (!forWriting)
                {
                    *errorCode = ErrorVariableNotInitialized;
                    return NULL;
                }
                variable = var_createSimpleVariable(interpreter, errorCode, symbolIndex, interpreter->subLevel, varType, NULL);
                if (!variable) return NULL;
            }
            if (variable->isReference)
            {
                return variable->v.reference;
            }
            return &variable->v;
        }
    }
    return &ValueDummy;
}

enum ErrorCode itp_checkTypeClass(struct Interpreter *interpreter, enum ValueType valueType, enum TypeClass typeClass)
{
    if (interpreter->pass == PassPrepare && valueType != ValueTypeError)
    {
        if (typeClass == TypeClassString && valueType != ValueTypeString)
        {
            return ErrorTypeMismatch;
        }
        else if (typeClass == TypeClassNumeric && valueType != ValueTypeFloat)
        {
            return ErrorTypeMismatch;
        }
    }
    return ErrorNone;
}

struct TypedValue itp_evaluateExpression(struct Core *core, enum TypeClass typeClass)
{
    struct TypedValue value = itp_evaluateExpressionLevel(core, 0);
    if (value.type != ValueTypeError)
    {
        enum ErrorCode errorCode = itp_checkTypeClass(core->interpreter, value.type, typeClass);
        if (errorCode != ErrorNone)
        {
            value.type = ValueTypeError;
            value.v.errorCode = errorCode;
        }
    }
    return value;
}

struct TypedValue itp_evaluateNumericExpression(struct Core *core, int min, int max)
{
    struct TypedValue value = itp_evaluateExpressionLevel(core, 0);
    if (value.type != ValueTypeError)
    {
        enum ErrorCode errorCode = ErrorNone;
        if (core->interpreter->pass == PassPrepare)
        {
            if (value.type != ValueTypeFloat)
            {
                errorCode = ErrorTypeMismatch;
            }
        }
        else if (core->interpreter->pass == PassRun)
        {
            if ((int)value.v.floatValue < min || (int)value.v.floatValue > max)
            {
                errorCode = ErrorInvalidParameter;
            }
        }
        if (errorCode != ErrorNone)
        {
            value.type = ValueTypeError;
            value.v.errorCode = errorCode;
        }
    }
    return value;
}

struct TypedValue itp_evaluateOptionalExpression(struct Core *core, enum TypeClass typeClass)
{
    if (core->interpreter->pc->type == TokenComma || core->interpreter->pc->type == TokenBracketClose || itp_isEndOfCommand(core->interpreter))
    {
        struct TypedValue value;
        value.type = ValueTypeNull;
        return value;
    }
    return itp_evaluateExpression(core, typeClass);
}

struct TypedValue itp_evaluateOptionalNumericExpression(struct Core *core, int min, int max)
{
    if (core->interpreter->pc->type == TokenComma || core->interpreter->pc->type == TokenBracketClose || itp_isEndOfCommand(core->interpreter))
    {
        struct TypedValue value;
        value.type = ValueTypeNull;
        return value;
    }
    return itp_evaluateNumericExpression(core, min, max);
}

bool itp_isTokenLevel(enum TokenType token, int level)
{
    switch (level)
    {
        case 0:
            return token == TokenXOR || token == TokenOR;
        case 1:
            return token == TokenAND;
//        case 2:
//            return token == TokenNOT;
        case 3:
            return token == TokenEq || token == TokenUneq || token == TokenGr || token == TokenLe || token == TokenGrEq || token == TokenLeEq;
        case 4:
            return token == TokenPlus || token == TokenMinus;
        case 5:
            return token == TokenMOD;
        case 6:
            return token == TokenMul || token == TokenDiv || token == TokenDivInt;
//        case 7:
//            return token == TokenPlus || token == TokenMinus; // unary
        case 8:
            return token == TokenPow;
    }
    return false;
}

struct TypedValue itp_evaluateExpressionLevel(struct Core *core, int level)
{
    struct Interpreter *interpreter = core->interpreter;
    enum TokenType type = interpreter->pc->type;
    
    if (level == 2 && type == TokenNOT)
    {
        ++interpreter->pc;
        ++interpreter->cycles;
        struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
        if (value.type == ValueTypeError) return value;
        enum ErrorCode errorCode = itp_checkTypeClass(core->interpreter, value.type, TypeClassNumeric);
        if (errorCode != ErrorNone)
        {
            value.type = ValueTypeError;
            value.v.errorCode = errorCode;
        }
        else
        {
            value.v.floatValue = ~((int)value.v.floatValue);
        }
        interpreter->lastVariableValue = NULL;
        return value;
    }
    if (level == 7 && (type == TokenPlus || type == TokenMinus)) // unary
    {
        ++interpreter->pc;
        ++interpreter->cycles;
        struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
        if (value.type == ValueTypeError) return value;
        enum ErrorCode errorCode = itp_checkTypeClass(core->interpreter, value.type, TypeClassNumeric);
        if (errorCode != ErrorNone)
        {
            value.type = ValueTypeError;
            value.v.errorCode = errorCode;
        }
        else if (type == TokenMinus)
        {
            value.v.floatValue = -value.v.floatValue;
        }
        interpreter->lastVariableValue = NULL;
        return value;
    }
    if (level == 9)
    {
        return itp_evaluatePrimaryExpression(core);
    }
    
    struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
    if (value.type == ValueTypeError) return value;
    
    while (itp_isTokenLevel(interpreter->pc->type, level))
    {
        enum TokenType type = interpreter->pc->type;
        ++interpreter->pc;
        ++interpreter->cycles;
        struct TypedValue rightValue = itp_evaluateExpressionLevel(core, level + 1);
        if (rightValue.type == ValueTypeError) return rightValue;
        
        struct TypedValue newValue;
        if (value.type != rightValue.type)
        {
            newValue.type = ValueTypeError;
            newValue.v.errorCode = ErrorTypeMismatch;
            return newValue;
        }
        
        if (value.type == ValueTypeFloat)
        {
            newValue.type = ValueTypeFloat;
            switch (type)
            {
                case TokenXOR: {
                    int leftInt = value.v.floatValue;
                    int rightInt = rightValue.v.floatValue;
                    newValue.v.floatValue = (leftInt ^ rightInt);
                    break;
                }
                case TokenOR: {
                    int leftInt = value.v.floatValue;
                    int rightInt = rightValue.v.floatValue;
                    newValue.v.floatValue = (leftInt | rightInt);
                    break;
                }
                case TokenAND: {
                    int leftInt = value.v.floatValue;
                    int rightInt = rightValue.v.floatValue;
                    newValue.v.floatValue = (leftInt & rightInt);
                    break;
                }
                case TokenEq: {
                    newValue.v.floatValue = (value.v.floatValue == rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
                    break;
                }
                case TokenUneq: {
                    newValue.v.floatValue = (value.v.floatValue != rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
                    break;
                }
                case TokenGr: {
                    newValue.v.floatValue = (value.v.floatValue > rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
                    break;
                }
                case TokenLe: {
                    newValue.v.floatValue = (value.v.floatValue < rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
                    break;
                }
                case TokenGrEq: {
                    newValue.v.floatValue = (value.v.floatValue >= rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
                    break;
                }
                case TokenLeEq: {
                    newValue.v.floatValue = (value.v.floatValue <= rightValue.v.floatValue) ? BAS_TRUE : BAS_FALSE;
                    break;
                }
                case TokenPlus: {
                    newValue.v.floatValue = value.v.floatValue + rightValue.v.floatValue;
                    break;
                }
                case TokenMinus: {
                    newValue.v.floatValue = value.v.floatValue - rightValue.v.floatValue;
                    break;
                }
                case TokenMOD: {
                    if (interpreter->pass == PassRun)
                    {
                        int rightInt = (int)rightValue.v.floatValue;
                        if (rightInt == 0)
                        {
                            newValue.type = ValueTypeError;
                            newValue.v.errorCode = ErrorDivisionByZero;
                        }
                        else
                        {
                            newValue.v.floatValue = (int)value.v.floatValue % rightInt;
                        }
                    }
                    break;
                }
                case TokenMul: {
                    newValue.v.floatValue = value.v.floatValue * rightValue.v.floatValue;
                    break;
                }
                case TokenDiv: {
                    if (interpreter->pass == PassRun)
                    {
                        if (rightValue.v.floatValue == 0.0f)
                        {
                            newValue.type = ValueTypeError;
                            newValue.v.errorCode = ErrorDivisionByZero;
                        }
                        else
                        {
                            newValue.v.floatValue = value.v.floatValue / rightValue.v.floatValue;
                        }
                    }
                    break;
                }
                case TokenDivInt: {
                    if (interpreter->pass == PassRun)
                    {
                        int rightInt = (int)rightValue.v.floatValue;
                        if (rightInt == 0)
                        {
                            newValue.type = ValueTypeError;
                            newValue.v.errorCode = ErrorDivisionByZero;
                        }
                        else
                        {
                            newValue.v.floatValue = (int)value.v.floatValue / rightInt;
                        }
                    }
                    break;
                }
                case TokenPow: {
                    newValue.v.floatValue = powf(value.v.floatValue, rightValue.v.floatValue);
                    break;
                }
                default: {
                    newValue.type = ValueTypeError;
                    newValue.v.errorCode = ErrorSyntax;
                }
            }
        }
        else if (value.type == ValueTypeString)
        {
            switch (type)
            {
                case TokenEq: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) == 0) ? BAS_TRUE : BAS_FALSE;
                    }
                    break;
                }
                case TokenUneq: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) != 0) ? BAS_TRUE : BAS_FALSE;
                    }
                    break;
                }
                case TokenGr: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) > 0) ? BAS_TRUE : BAS_FALSE;
                    }
                    break;
                }
                case TokenLe: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) < 0) ? BAS_TRUE : BAS_FALSE;
                    }
                    break;
                }
                case TokenGrEq: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) >= 0) ? BAS_TRUE : BAS_FALSE;
                    }
                    break;
                }
                case TokenLeEq: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) <= 0) ? BAS_TRUE : BAS_FALSE;
                    }
                    break;
                }
                case TokenPlus: {
                    newValue.type = ValueTypeString;
                    if (interpreter->pass == PassRun)
                    {
                        size_t len1 = strlen(value.v.stringValue->chars);
                        size_t len2 = strlen(rightValue.v.stringValue->chars);
                        newValue.v.stringValue = rcstring_new(NULL, len1 + len2);
                        strcpy(newValue.v.stringValue->chars, value.v.stringValue->chars);
                        strcpy(&newValue.v.stringValue->chars[len1], rightValue.v.stringValue->chars);
                        interpreter->cycles += len1 + len2;
                    }
                    break;
                }
                case TokenXOR:
                case TokenOR:
                case TokenAND:
                case TokenMinus:
                case TokenMOD:
                case TokenMul:
                case TokenDiv:
                case TokenDivInt:
                case TokenPow: {
                    newValue.type = ValueTypeError;
                    newValue.v.errorCode = ErrorTypeMismatch;
                    break;
                }
                default: {
                    newValue.type = ValueTypeError;
                    newValue.v.errorCode = ErrorSyntax;
                }
            }
            if (interpreter->pass == PassRun)
            {
                rcstring_release(value.v.stringValue);
                rcstring_release(rightValue.v.stringValue);
            }
        }
        else
        {
            assert(0);
            newValue.v.floatValue = 0;
        }
        
        value = newValue;
        interpreter->lastVariableValue = NULL;
        if (value.type == ValueTypeError) break;
    }
    return value;
}

struct TypedValue itp_evaluatePrimaryExpression(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // check for function
    struct TypedValue value = itp_evaluateFunction(core);
    if (value.type != ValueTypeNull)
    {
        ++interpreter->cycles;
        interpreter->lastVariableValue = NULL;
        return value;
    }
    
    interpreter->lastVariableValue = NULL;
    
    // native types
    switch (interpreter->pc->type)
    {
        case TokenFloat: {
            value.type = ValueTypeFloat;
            value.v.floatValue = interpreter->pc->floatValue;
            ++interpreter->pc;
            ++interpreter->cycles;
            break;
        }
        case TokenString: {
            value.type = ValueTypeString;
            value.v.stringValue = interpreter->pc->stringValue;
            if (interpreter->pass == PassRun)
            {
                rcstring_retain(interpreter->pc->stringValue);
            }
            ++interpreter->pc;
            ++interpreter->cycles;
            break;
        }
        case TokenIdentifier:
        case TokenStringIdentifier: {
            enum ErrorCode errorCode = ErrorNone;
            enum ValueType valueType = ValueTypeNull;
            union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
            if (varValue)
            {
                value.type = valueType;
                value.v = *varValue;
                interpreter->lastVariableValue = varValue;
                if (interpreter->pass == PassRun && valueType == ValueTypeString)
                {
                    rcstring_retain(varValue->stringValue);
                }
            }
            else
            {
                value.type = ValueTypeError;
                value.v.errorCode = errorCode;
            }
            break;
        }
        case TokenBracketOpen: {
            ++interpreter->pc;
            value = itp_evaluateExpression(core, TypeClassAny);
            if (value.type == ValueTypeError) return value;
            if (interpreter->pc->type != TokenBracketClose)
            {
                value.type = ValueTypeError;
                value.v.errorCode = ErrorSyntax;
            }
            else
            {
                ++interpreter->pc;
                interpreter->lastVariableValue = NULL;
            }
            break;
        }
        default: {
            value.type = ValueTypeError;
            value.v.errorCode = ErrorSyntax;
        }
    }
    return value;
}

bool itp_isEndOfCommand(struct Interpreter *interpreter)
{
    enum TokenType type = interpreter->pc->type;
    return (type == TokenEol || type == TokenELSE);
}

enum ErrorCode itp_endOfCommand(struct Interpreter *interpreter)
{
    enum TokenType type = interpreter->pc->type;
    if (type == TokenEol)
    {
        interpreter->isSingleLineIf = false;
        ++interpreter->pc;
        return ErrorNone;
    }
    return (type == TokenELSE) ? ErrorNone : ErrorSyntax;
}

enum TokenType itp_getNextTokenType(struct Interpreter *interpreter)
{
    return (interpreter->pc + 1)->type;
}

struct TypedValue itp_evaluateFunction(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    switch (interpreter->pc->type)
    {
        case TokenASC:
            return fnc_ASC(core);
            
        case TokenBIN:
        case TokenHEX:
            return fnc_BIN_HEX(core);
            
        case TokenCHR:
            return fnc_CHR(core);
            
        case TokenINSTR:
            return fnc_INSTR(core);
            
        case TokenLEFTStr:
        case TokenRIGHTStr:
            return fnc_LEFTStr_RIGHTStr(core);
            
        case TokenLEN:
            return fnc_LEN(core);
            
        case TokenMID:
            return fnc_MID(core);
            
        case TokenSTR:
            return fnc_STR(core);
            
        case TokenVAL:
            return fnc_VAL(core);

        case TokenPEEK:
        case TokenPEEKW:
        case TokenPEEKL:
            return fnc_PEEK(core);
            
        case TokenPI:
            return fnc_math0(core);

        case TokenABS:
        case TokenACOS:
        case TokenASIN:
        case TokenATAN:
        case TokenCOS:
        case TokenEXP:
        case TokenHCOS:
        case TokenHSIN:
        case TokenHTAN:
        case TokenINT:
        case TokenLOG:
        case TokenSGN:
        case TokenSIN:
        case TokenSQR:
        case TokenTAN:
            return fnc_math1(core);

        case TokenMAX:
        case TokenMIN:
            return fnc_math2(core);
            
        case TokenRND:
            return fnc_RND(core);
            
        case TokenINKEY:
            return fnc_INKEY(core);
        
        case TokenROM:
        case TokenSIZE:
            return fnc_ROM_SIZE(core);
            
        case TokenCOLOR:
            return fnc_COLOR(core);
            
        case TokenTIMER:
        case TokenRASTER:
        case TokenDISPLAY:
            return fnc_screen0(core);
            
        case TokenSCROLLX:
        case TokenSCROLLY:
            return fnc_SCROLL_X_Y(core);
            
        case TokenCELLA:
        case TokenCELLC:
            return fnc_CELL(core);
            
        case TokenMCELLA:
        case TokenMCELLC:
            return fnc_MCELL(core);
            
        case TokenUP:
        case TokenDOWN:
        case TokenLEFT:
        case TokenRIGHT:
            return fnc_UP_DOWN_LEFT_RIGHT(core);
            
        case TokenBUTTON:
            return fnc_BUTTON(core);
            
        case TokenSPRITEX:
        case TokenSPRITEY:
        case TokenSPRITEC:
        case TokenSPRITEA:
            return fnc_SPRITE(core);
            
        case TokenSPRITE:
            return fnc_SPRITE_HIT(core);
            
        case TokenHIT:
            return fnc_HIT(core);
            
        case TokenTOUCH:
            return fnc_TOUCH(core);

        case TokenTAP:
            return fnc_TAP(core);
            
        case TokenTOUCHX:
        case TokenTOUCHY:
            return fnc_TOUCH_X_Y(core);
            
        case TokenFILE:
            return fnc_FILE(core);
            
        case TokenFSIZE:
            return fnc_FSIZE(core);
            
        case TokenPAUSE:
            return fnc_PAUSE(core);
            
        case TokenMUSIC:
            return fnc_MUSIC(core);
            
        default:
            break;
    }
    struct TypedValue value;
    value.type = ValueTypeNull;
    return value;
}

enum ErrorCode itp_evaluateCommand(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    enum TokenType type = interpreter->pc->type;
    if (type != TokenREM && type != TokenApostrophe && type != TokenEol && type != TokenUndefined)
    {
        ++interpreter->cycles;
    }
    switch (type)
    {
        case TokenUndefined:
            if (interpreter->pass == PassRun)
            {
                itp_endProgram(core);
            }
            break;
            
        case TokenREM:
        case TokenApostrophe:
            ++interpreter->pc;
            break;
            
        case TokenLabel:
            ++interpreter->pc;
            if (interpreter->pc->type != TokenEol) return ErrorSyntax;
            ++interpreter->pc;
            break;
        
        case TokenEol:
            interpreter->isSingleLineIf = false;
            ++interpreter->pc;
            break;
            
        case TokenEND:
            switch (itp_getNextTokenType(interpreter))
            {
                case TokenIF:
                    return cmd_END_IF(core);
                    
                case TokenSUB:
                    return cmd_END_SUB(core);
                    
                default:
                    return cmd_END(core);
            }
            break;
            
        case TokenLET:
        case TokenIdentifier:
        case TokenStringIdentifier:
            return cmd_LET(core);
            
        case TokenDIM:
            return cmd_DIM(core);
        
        case TokenPRINT:
            return cmd_PRINT(core);
            
        case TokenCLS:
            return cmd_CLS(core);
            
        case TokenINPUT:
            return cmd_INPUT(core);
        
        case TokenIF:
            return cmd_IF(core, false);
        
        case TokenELSE:
            return cmd_ELSE(core);

        case TokenFOR:
            return cmd_FOR(core);

        case TokenNEXT:
            return cmd_NEXT(core);

        case TokenGOTO:
            return cmd_GOTO(core);

        case TokenGOSUB:
            return cmd_GOSUB(core);
            
        case TokenRETURN:
            return cmd_RETURN(core);
            
        case TokenDATA:
            return cmd_DATA(core);

        case TokenREAD:
            return cmd_READ(core);

        case TokenRESTORE:
            return cmd_RESTORE(core);

        case TokenPOKE:
        case TokenPOKEW:
        case TokenPOKEL:
            return cmd_POKE(core);
            
        case TokenFILL:
            return cmd_FILL(core);
            
        case TokenCOPY:
            return cmd_COPY(core);
            
        case TokenROL:
        case TokenROR:
            return cmd_ROL_ROR(core);
            
        case TokenWAIT:
            return cmd_WAIT(core);
            
        case TokenON:
            return cmd_ON(core);
            
        case TokenSWAP:
            return cmd_SWAP(core);
            
        case TokenTEXT:
            return cmd_TEXT(core);

        case TokenNUMBER:
            return cmd_NUMBER(core);
            
        case TokenDO:
            return cmd_DO(core);
            
        case TokenLOOP:
            return cmd_LOOP(core);
        
        case TokenREPEAT:
            return cmd_REPEAT(core);
            
        case TokenUNTIL:
            return cmd_UNTIL(core);
            
        case TokenWHILE:
            return cmd_WHILE(core);
            
        case TokenWEND:
            return cmd_WEND(core);
            
        case TokenSYSTEM:
            return cmd_SYSTEM(core);

        case TokenRANDOMIZE:
            return cmd_RANDOMIZE(core);
            
        case TokenADD:
            return cmd_ADD(core);
            
        case TokenINC:
        case TokenDEC:
            return cmd_INC_DEC(core);
            
        case TokenLEFTStr:
        case TokenRIGHTStr:
            return cmd_LEFT_RIGHT(core);
            
        case TokenMID:
            return cmd_MID(core);
            
        case TokenWINDOW:
            return cmd_WINDOW(core);
            
        case TokenFONT:
            return cmd_FONT(core);
            
        case TokenLOCATE:
            return cmd_LOCATE(core);
            
        case TokenCLW:
            return cmd_CLW(core);
            
        case TokenBG:
            switch (itp_getNextTokenType(interpreter))
            {
                case TokenSOURCE:
                    return cmd_BG_SOURCE(core);
                    
                case TokenCOPY:
                    return cmd_BG_COPY(core);
                    
                case TokenSCROLL:
                    return cmd_BG_SCROLL(core);
                    
                case TokenFILL:
                    return cmd_BG_FILL(core);
                    
                case TokenTINT:
                    return cmd_BG_TINT(core);
                    
                case TokenVIEW:
                    return cmd_BG_VIEW(core);
                    
                default:
                    return cmd_BG(core);
            }
            break;
            
        case TokenATTR:
            return cmd_ATTR(core);
            
        case TokenPAL:
            return cmd_PAL(core);
            
        case TokenFLIP:
            return cmd_FLIP(core);
            
        case TokenPRIO:
            return cmd_PRIO(core);
            
        case TokenCELL:
            switch (itp_getNextTokenType(interpreter))
            {
                case TokenSIZE:
                    return cmd_CELL_SIZE(core);
                    
                default:
                    return cmd_CELL(core);
            }
            break;
            
        case TokenTINT:
            return cmd_TINT(core);
            
        case TokenMCELL:
            return cmd_MCELL(core);
            
        case TokenPALETTE:
            return cmd_PALETTE(core);
            
        case TokenSCROLL:
            return cmd_SCROLL(core);

        case TokenDISPLAY:
            return cmd_DISPLAY(core);
            
        case TokenSPRITEA:
            return cmd_SPRITE_A(core);
            
        case TokenSPRITE:
            switch (itp_getNextTokenType(interpreter))
            {
                case TokenOFF:
                    return cmd_SPRITE_OFF(core);
                    
                case TokenVIEW:
                    return cmd_SPRITE_VIEW(core);
                    
                default:
                    return cmd_SPRITE(core);
            }
            break;
            
        case TokenSAVE:
            return cmd_SAVE(core);
            
        case TokenLOAD:
            return cmd_LOAD(core);
            
        case TokenFILES:
            return cmd_FILES(core);
            
        case TokenGAMEPAD:
            return cmd_GAMEPAD(core);
            
        case TokenKEYBOARD:
            return cmd_KEYBOARD(core);
            
        case TokenTOUCHSCREEN:
            return cmd_TOUCHSCREEN(core);
            
        case TokenTRACE:
            return cmd_TRACE(core);
            
        case TokenCALL:
            return cmd_CALL(core);
            
        case TokenSUB:
            return cmd_SUB(core);
            
//        case TokenSHARED:
//            return cmd_SHARED(core);
            
        case TokenGLOBAL:
            return cmd_GLOBAL(core);
            
        case TokenEXIT:
            return cmd_EXIT_SUB(core);
            
        case TokenPAUSE:
            return cmd_PAUSE(core);
            
        case TokenSOUND:
            switch (itp_getNextTokenType(interpreter))
            {
//                case TokenCOPY:
//                    return cmd_SOUND_COPY(core);
                    
                case TokenSOURCE:
                    return cmd_SOUND_SOURCE(core);
                    
                default:
                    return cmd_SOUND(core);
            }
            break;
            
        case TokenVOLUME:
            return cmd_VOLUME(core);
            
        case TokenENVELOPE:
            return cmd_ENVELOPE(core);
            
        case TokenLFO:
            switch (itp_getNextTokenType(interpreter))
            {
                case TokenWAVE:
                    return cmd_LFO_WAVE(core);
                    
                default:
                    return cmd_LFO(core);
            }
            break;
            
        case TokenLFOA:
            return cmd_LFO_A(core);
            
        case TokenPLAY:
            return cmd_PLAY(core);
            
        case TokenSTOP:
            return cmd_STOP(core);
            
        case TokenMUSIC:
            return cmd_MUSIC(core);
            
        case TokenTRACK:
            return cmd_TRACK(core);
            
        default:
            printf("Command not implemented: %s\n", TokenStrings[interpreter->pc->type]);
            return ErrorSyntax;
    }
    return ErrorNone;
}
