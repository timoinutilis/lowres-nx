//
// Copyright 2016-2017 Timo Kloss
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
#include "core.h"
#include "cmd_control.h"
#include "cmd_variables.h"
#include "cmd_data.h"
#include "cmd_strings.h"
#include "cmd_memory.h"
#include "cmd_text.h"
#include "cmd_maths.h"

enum ErrorCode itp_tokenizeProgram(struct Core *core, const char *sourceCode);
struct TypedValue itp_evaluateExpressionLevel(struct Core *core, int level);
struct TypedValue itp_evaluatePrimaryExpression(struct Core *core);
struct TypedValue itp_evaluateFunction(struct Core *core);
enum ErrorCode itp_evaluateCommand(struct Core *core);

enum ErrorCode itp_compileProgram(struct Core *core, const char *sourceCode)
{
    // Tokenize
    
    enum ErrorCode errorCode = itp_tokenizeProgram(core, sourceCode);
    if (errorCode != ErrorNone) return errorCode;
    
    struct Interpreter *interpreter = &core->interpreter;
    
    // Prepare
    
    interpreter->pc = interpreter->tokens;
    interpreter->pass = PassPrepare;
    
    do
    {
        errorCode = itp_evaluateCommand(core);
    }
    while (errorCode == ErrorNone && interpreter->pc->type != TokenUndefined);
    
    if (errorCode != ErrorNone) return errorCode;
    assert(interpreter->numLabelStackItems == 0);
    
    // global null string
    interpreter->nullString = rcstring_new(NULL, 0);
    
    itp_resetProgram(core);
    
    return ErrorNone;
}

void itp_resetProgram(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    interpreter->pc = interpreter->tokens;
    interpreter->pass = PassRun;
    interpreter->state = StateEvaluate;
    interpreter->mode = ModeNone;
    interpreter->exitErrorCode = ErrorNone;
    interpreter->numLabelStackItems = 0;
    interpreter->isSingleLineIf = false;
    interpreter->numSimpleVariables = 0;
    interpreter->numArrayVariables = 0;
    interpreter->currentDataToken = interpreter->firstData;
    interpreter->currentDataValueToken = interpreter->firstData + 1;
}

void itp_runProgram(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    switch (interpreter->state)
    {
        case StateEvaluate:
        {
            interpreter->mode = ModeMain;
            interpreter->exitEvaluation = false;
            enum ErrorCode errorCode = ErrorNone;
            int cycles = 0;
            
            do
            {
                errorCode = itp_evaluateCommand(core);
                cycles++;
            }
            while (errorCode == ErrorNone && cycles < MAX_CYCLES_PER_FRAME && interpreter->state == StateEvaluate && !interpreter->exitEvaluation);
            
            interpreter->mode = ModeNone;
            if (cycles == MAX_CYCLES_PER_FRAME)
            {
                printf("Warning: Max cycles per frame reached.\n");
            }
            if (errorCode != ErrorNone)
            {
                interpreter->exitErrorCode = errorCode;
                interpreter->state = StateEnd;
            }
            break;
        }
            
        case StateWait:
        {
            if (interpreter->waitCount > 0)
            {
                --interpreter->waitCount;
            }
            else
            {
                interpreter->state = StateEvaluate;
            }
            break;
        }
            
        case StateInput:
        {
            if (txtlib_inputUpdate(core))
            {
                interpreter->state = StateEvaluate;
                cmd_endINPUT(core);
            }
            break;
        }
            
        case StateNoProgram:
        case StateEnd:
            break;
    }
}

void itp_runInterrupt(struct Core *core, enum InterruptType type)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    switch (interpreter->state)
    {
        case StateEvaluate:
        case StateWait:
        case StateInput:
        {
            struct Token *startToken;
            int maxCycles;
            switch (type)
            {
                case InterruptTypeRaster:
                    startToken = interpreter->currentOnRasterToken;
                    maxCycles = MAX_CYCLES_PER_RASTER;
                    break;
                    
                case InterruptTypeVBL:
                    startToken = interpreter->currentOnVBLToken;
                    maxCycles = MAX_CYCLES_PER_VBL;
                    break;
            }
            
            if (startToken)
            {
                interpreter->mode = ModeInterrupt;
                interpreter->exitEvaluation = false;
                struct Token *pc = interpreter->pc;
                interpreter->pc = startToken;
                
                enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeONGOSUB, NULL);
                int cycles = 0;
                
                while (errorCode == ErrorNone && cycles < maxCycles && !interpreter->exitEvaluation)
                {
                    errorCode = itp_evaluateCommand(core);
                    cycles++;
                }
                
                interpreter->mode = ModeNone;
                if (cycles == maxCycles)
                {
                    interpreter->exitErrorCode = ErrorTooManyCommandCycles;
                    interpreter->state = StateEnd;
                }
                else if (errorCode != ErrorNone)
                {
                    interpreter->exitErrorCode = errorCode;
                    interpreter->state = StateEnd;
                }
                else
                {
                    interpreter->pc = pc;
                }
            }
            break;
        }
            
        case StateNoProgram:
        case StateEnd:
            break;
    }

}

void itp_freeProgram(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    interpreter->state = StateNoProgram;
    
    var_freeSimpleVariables(interpreter);
    var_freeArrayVariables(interpreter);
    
    // Free string tokens
    for (int i = 0; i < interpreter->numTokens; i++)
    {
        struct Token *token = &interpreter->tokens[i];
        if (token->type == TokenString)
        {
            rcstring_release(token->stringValue);
        }
    }
    
    // Free null string
    if (interpreter->nullString)
    {
        rcstring_release(interpreter->nullString);
    }
    
    assert(rcstring_count == 0);
}

enum ErrorCode itp_tokenizeProgram(struct Core *core, const char *sourceCode)
{
    const char *charSetDigits = "0123456789";
    const char *charSetLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    const char *charSetAlphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
    const char *charSetHex = "0123456789ABCDEF";
    
    struct Interpreter *interpreter = &core->interpreter;
    const char *character = sourceCode;
    
    // PROGRAM
    
    while (*character && *character != '#')
    {
        if (interpreter->numTokens >= MAX_TOKENS)
        {
            return ErrorTooManyTokens;
        }
        struct Token *token = &interpreter->tokens[interpreter->numTokens];
        token->sourcePosition = (int)(character - sourceCode);
        interpreter->pc = token; // for error handling
        
        // line break
        if (*character == '\n')
        {
            token->type = TokenEol;
            interpreter->numTokens++;
            character++;
            continue;
        }
        
        // space
        if (*character == ' ' || *character == '\t')
        {
            character++;
            continue;
        }
        
        // string
        if (*character == '"')
        {
            character++;
            const char *firstCharacter = character;
            while (*character && *character != '"')
            {
                character++;
                if (*character == '\n')
                {
                    return ErrorUnterminatedString;
                }
            }
            int len = (int)(character - firstCharacter);
            struct RCString *string = rcstring_new(firstCharacter, len);
            if (!string) return ErrorOutOfMemory;
            token->type = TokenString;
            token->stringValue = string;
            interpreter->numTokens++;
            character++;
            continue;
        }
        
        // number
        if (strchr(charSetDigits, *character))
        {
            float number = 0;
            int afterDot = 0;
            while (*character)
            {
                if (strchr(charSetDigits, *character))
                {
                    int digit = (int)*character - (int)'0';
                    if (afterDot == 0)
                    {
                        number *= 10;
                        number += digit;
                    }
                    else
                    {
                        number += (float)digit / afterDot;
                        afterDot *= 10;
                    }
                    character++;
                }
                else if (*character == '.' && afterDot == 0)
                {
                    afterDot = 10;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            interpreter->numTokens++;
            continue;
        }
        
        // hex number
        if (*character == '$')
        {
            character++;
            int number = 0;
            while (*character)
            {
                char *spos = strchr(charSetHex, *character);
                if (spos)
                {
                    int digit = (int)(spos - charSetHex);
                    number <<= 4;
                    number += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            interpreter->numTokens++;
            continue;
        }
        
        // Keyword
        enum TokenType foundKeywordToken = TokenUndefined;
        for (int i = 0; i < Token_count; i++)
        {
            const char *keyword = TokenStrings[i];
            if (keyword)
            {
                size_t keywordLen = strlen(keyword);
                int keywordIsAlphaNum = strchr(charSetAlphaNum, keyword[0]) != NULL;
                for (int pos = 0; pos <= keywordLen && character[pos]; pos++)
                {
                    char textCharacter = character[pos];
                    
                    if (pos < keywordLen)
                    {
                        char symbCharacter = keyword[pos];
                        if (symbCharacter != textCharacter)
                        {
                            // not matching
                            break;
                        }
                    }
                    else if (keywordIsAlphaNum && strchr(charSetAlphaNum, textCharacter))
                    {
                        // matching, but word is longer, so seems to be an identifier
                        break;
                    }
                    else
                    {
                        // symbol found!
                        foundKeywordToken = i;
                        character += keywordLen;
                        break;
                    }
                }
                if (foundKeywordToken != TokenUndefined)
                {
                    break;
                }
            }
        }
        if (foundKeywordToken != TokenUndefined)
        {
            if (foundKeywordToken == TokenREM)
            {
                // REM comment, skip until end of line
                while (*character)
                {
                    character++;
                    if (*character == '\n')
                    {
                        character++;
                        break;
                    }
                }
            }
            else if (foundKeywordToken > Token_reserved)
            {
                return ErrorReservedKeyword;
            }
            token->type = foundKeywordToken;
            interpreter->numTokens++;
            continue;
        }
        
        // Symbol
        if (strchr(charSetLetters, *character))
        {
            const char *firstCharacter = character;
            char isString = 0;
            while (*character)
            {
                if (strchr(charSetAlphaNum, *character))
                {
                    character++;
                }
                else
                {
                    if (*character == '$')
                    {
                        isString = 1;
                        character++;
                    }
                    break;
                }
            }
            if (interpreter->numSymbols >= MAX_SYMBOLS)
            {
                return ErrorTooManySymbols;
            }
            int len = (int)(character - firstCharacter);
            if (len >= SYMBOL_NAME_SIZE)
            {
                return ErrorSymbolNameTooLong;
            }
            char symbolName[SYMBOL_NAME_SIZE];
            memcpy(symbolName, firstCharacter, len);
            symbolName[len] = 0;
            int symbolIndex = -1;
            // find existing symbol
            for (int i = 0; i < MAX_SYMBOLS && interpreter->symbols[i].name[0] != 0; i++)
            {
                if (strcmp(symbolName, interpreter->symbols[i].name) == 0)
                {
                    symbolIndex = i;
                    break;
                }
            }
            if (symbolIndex == -1)
            {
                // add new symbol
                strcpy(interpreter->symbols[interpreter->numSymbols].name, symbolName);
                symbolIndex = interpreter->numSymbols++;
                printf("symbol %d: %s\n", symbolIndex, symbolName);
            }
            if (isString)
            {
                token->type = TokenStringIdentifier;
            }
            else if (*character == ':')
            {
                token->type = TokenLabel;
                character++;
                enum ErrorCode errorCode = lab_setJumpLabel(interpreter, symbolIndex, token + 1);
                if (errorCode != ErrorNone) return errorCode;
            }
            else
            {
                token->type = TokenIdentifier;
            }
            token->symbolIndex = symbolIndex;
            interpreter->numTokens++;
            continue;
        }
        
        // Unexpected character
        return ErrorUnexpectedCharacter;
    }
    
    // ROM DATA
    
    struct RomDataEntry *romDataEntries = (struct RomDataEntry *)core->machine.cartridgeRom;
    
    uint8_t *currentRomByte = (uint8_t *)&romDataEntries[MAX_ROM_DATA_ENTRIES]; // after entries
    uint8_t *endRomByte = &core->machine.cartridgeRom[0x8000];
    
    while (*character)
    {
        if (*character == '#')
        {
            character++;
            
            // entry index
            int entryIndex = 0;
            while (*character)
            {
                if (strchr(charSetDigits, *character))
                {
                    int digit = (int)*character - (int)'0';
                    entryIndex *= 10;
                    entryIndex += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            if (*character != ':') return ErrorUnexpectedCharacter;
            
            if (entryIndex >= MAX_ROM_DATA_ENTRIES) return ErrorIndexOutOfBounds;
            if (BigEndianUInt16_get(&romDataEntries[entryIndex].length) > 0) return ErrorIndexAlreadyDefined;
            
            // skip until end of line
            do
            {
                character++;
            }
            while (*character && *character != '\n');
            
            // binary data
            uint8_t *startByte = currentRomByte;
            bool shift = true;
            int value = 0;
            while (*character && *character != '#')
            {
                char *spos = strchr(charSetHex, *character);
                if (spos)
                {
                    int digit = (int)(spos - charSetHex);
                    if (shift)
                    {
                        value = digit << 4;
                    }
                    else
                    {
                        value |= digit;
                        if (currentRomByte >= endRomByte) return ErrorRomIsFull;
                        *currentRomByte = value;
                        ++currentRomByte;
                    }
                    shift = !shift;
                }
                else if (*character != ' ' && *character == '\t' && *character == '\n')
                {
                    return ErrorUnexpectedCharacter;
                }
                character++;
            }
            if (!shift) return ErrorSyntax; // incomplete hex value
            
            int start = (int)(startByte - core->machine.cartridgeRom);
            int length = (int)(currentRomByte - startByte);
            struct RomDataEntry *entry = &romDataEntries[entryIndex];
            BigEndianUInt16_set(&entry->start, start);
            BigEndianUInt16_set(&entry->length, length);
            printf("index %d: start=%d length=%d\n", entryIndex, start, length);
        }
        else if (*character == ' ' || *character == '\t' || *character == '\n')
        {
            character++;
        }
        else
        {
            return ErrorUnexpectedCharacter;
        }
    }
    
    return ErrorNone;
}

union Value *itp_readVariable(struct Core *core, enum ValueType *type, enum ErrorCode *errorCode)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    struct Token *tokenIdentifier = interpreter->pc;
    
    if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
    {
        *errorCode = ErrorExpectedVariableIdentifier;
        return NULL;
    }
    
    enum ValueType varType = ValueTypeNull;
    if (tokenIdentifier->type == TokenIdentifier)
    {
        varType = ValueTypeFloat;
    }
    else if (tokenIdentifier->type == TokenStringIdentifier)
    {
        varType = ValueTypeString;
    }
    if (type)
    {
        *type = varType;
    }
    
    int symbolIndex = tokenIdentifier->symbolIndex;
    ++interpreter->pc;
    
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // array
        ++interpreter->pc;
        
        struct ArrayVariable *variable = NULL;
        if (interpreter->pass == PassRun)
        {
            variable = var_getArrayVariable(interpreter, symbolIndex);
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
            *errorCode = ErrorExpectedRightParenthesis;
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
            struct SimpleVariable *variable = var_getSimpleVariable(interpreter, errorCode, symbolIndex, varType);
            if (!variable) return NULL;
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
            return ErrorExpectedStringExpression;
        }
        else if (typeClass == TypeClassNumeric && valueType != ValueTypeFloat)
        {
            return ErrorExpectedNumericExpression;
        }
    }
    return ErrorNone;
}

struct TypedValue itp_evaluateExpression(struct Core *core, enum TypeClass typeClass)
{
    struct TypedValue value = itp_evaluateExpressionLevel(core, 0);
    enum ErrorCode errorCode = itp_checkTypeClass(&core->interpreter, value.type, typeClass);
    if (errorCode != ErrorNone)
    {
        value.type = ValueTypeError;
        value.v.errorCode = errorCode;
    }
    return value;
}

struct TypedValue itp_evaluateNumericExpression(struct Core *core, int min, int max)
{
    struct TypedValue value = itp_evaluateExpressionLevel(core, 0);
    if (value.type != ValueTypeError)
    {
        enum ErrorCode errorCode = ErrorNone;
        if (core->interpreter.pass == PassPrepare)
        {
            if (value.type != ValueTypeFloat)
            {
                errorCode = ErrorExpectedNumericExpression;
            }
        }
        else if (core->interpreter.pass == PassRun)
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
            return token == TokenMul || token == TokenDiv;
//        case 7:
//            return token == TokenPlus || token == TokenMinus; // unary
        case 8:
            return token == TokenPow;
    }
    return false;
}

struct TypedValue itp_evaluateExpressionLevel(struct Core *core, int level)
{
    struct Interpreter *interpreter = &core->interpreter;
    enum TokenType type = interpreter->pc->type;
    
    if (level == 2 && type == TokenNOT)
    {
        ++interpreter->pc;
        struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
        if (value.type == ValueTypeError) return value;
        enum ErrorCode errorCode = itp_checkTypeClass(&core->interpreter, value.type, TypeClassNumeric);
        if (errorCode != ErrorNone)
        {
            value.type = ValueTypeError;
            value.v.errorCode = errorCode;
        }
        else
        {
            value.v.floatValue = ~((int)value.v.floatValue);
        }
        return value;
    }
    if (level == 7 && (type == TokenPlus || type == TokenMinus)) // unary
    {
        ++interpreter->pc;
        struct TypedValue value = itp_evaluateExpressionLevel(core, level + 1);
        if (value.type == ValueTypeError) return value;
        enum ErrorCode errorCode = itp_checkTypeClass(&core->interpreter, value.type, TypeClassNumeric);
        if (errorCode != ErrorNone)
        {
            value.type = ValueTypeError;
            value.v.errorCode = errorCode;
        }
        else if (type == TokenMinus)
        {
            value.v.floatValue = -value.v.floatValue;
        }
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
                    newValue.v.floatValue = (value.v.floatValue == rightValue.v.floatValue) ? -1.0f : 0.0f;
                    break;
                }
                case TokenUneq: {
                    newValue.v.floatValue = (value.v.floatValue != rightValue.v.floatValue) ? -1.0f : 0.0f;
                    break;
                }
                case TokenGr: {
                    newValue.v.floatValue = (value.v.floatValue > rightValue.v.floatValue) ? -1.0f : 0.0f;
                    break;
                }
                case TokenLe: {
                    newValue.v.floatValue = (value.v.floatValue < rightValue.v.floatValue) ? -1.0f : 0.0f;
                    break;
                }
                case TokenGrEq: {
                    newValue.v.floatValue = (value.v.floatValue >= rightValue.v.floatValue) ? -1.0f : 0.0f;
                    break;
                }
                case TokenLeEq: {
                    newValue.v.floatValue = (value.v.floatValue <= rightValue.v.floatValue) ? -1.0f : 0.0f;
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
                    newValue.v.floatValue = (int)value.v.floatValue % (int)rightValue.v.floatValue;
                    break;
                }
                case TokenMul: {
                    newValue.v.floatValue = value.v.floatValue * rightValue.v.floatValue;
                    break;
                }
                case TokenDiv: {
                    newValue.v.floatValue = value.v.floatValue / rightValue.v.floatValue;
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
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) == 0) ? -1.0f : 0.0f;
                    }
                    break;
                }
                case TokenUneq: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) != 0) ? -1.0f : 0.0f;
                    }
                    break;
                }
                case TokenGr: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) > 0) ? -1.0f : 0.0f;
                    }
                    break;
                }
                case TokenLe: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) < 0) ? -1.0f : 0.0f;
                    }
                    break;
                }
                case TokenGrEq: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) >= 0) ? -1.0f : 0.0f;
                    }
                    break;
                }
                case TokenLeEq: {
                    newValue.type = ValueTypeFloat;
                    if (interpreter->pass == PassRun)
                    {
                        newValue.v.floatValue = (strcmp(value.v.stringValue->chars, rightValue.v.stringValue->chars) <= 0) ? -1.0f : 0.0f;
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
        
        value = newValue;
        if (value.type == ValueTypeError) break;
    }
    return value;
}

struct TypedValue itp_evaluatePrimaryExpression(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // check for function
    struct TypedValue value = itp_evaluateFunction(core);
    if (value.type != ValueTypeNull) return value;
    
    // native types
    switch (interpreter->pc->type)
    {
        case TokenFloat: {
            value.type = ValueTypeFloat;
            value.v.floatValue = interpreter->pc->floatValue;
            ++interpreter->pc;
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
            break;
        }
        case TokenIdentifier:
        case TokenStringIdentifier: {
            enum ErrorCode errorCode = ErrorNone;
            enum ValueType valueType = ValueTypeNull;
            union Value *varValue = itp_readVariable(core, &valueType, &errorCode);
            if (varValue)
            {
                value.type = valueType;
                value.v = *varValue;
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
            if (interpreter->pc->type != TokenBracketClose)
            {
                value.type = ValueTypeError;
                value.v.errorCode = ErrorExpectedRightParenthesis;
            }
            else
            {
                ++interpreter->pc;
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

int itp_isEndOfCommand(struct Interpreter *interpreter)
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
    return (type == TokenELSE) ? ErrorNone : ErrorUnexpectedToken;
}

enum TokenType itp_getNextTokenType(struct Interpreter *interpreter)
{
    return (interpreter->pc + 1)->type;
}

struct TypedValue itp_evaluateFunction(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
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
            
        case TokenLEFT:
        case TokenRIGHT:
            return fnc_LEFT_RIGHT(core);
            
        case TokenLEN:
            return fnc_LEN(core);
            
        case TokenMID:
            return fnc_MID(core);
            
        case TokenSTR:
            return fnc_STR(core);
            
        case TokenVAL:
            return fnc_VAL(core);

        case TokenPEEK:
            return fnc_PEEK(core);
            
        case TokenPI:
        case TokenRND:
        case TokenTIMER:
            return fnc_math0(core);
            
        case TokenABS:
        case TokenATN:
        case TokenCOS:
        case TokenEXP:
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
            
        case TokenINKEY:
            return fnc_INKEY(core);
        
        case TokenSTART:
        case TokenLENGTH:
            return fnc_START_LENGTH(core);

        default:
            break;
    }
    struct TypedValue value;
    value.type = ValueTypeNull;
    return value;
}

enum ErrorCode itp_evaluateCommand(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    switch (interpreter->pc->type)
    {
        case TokenUndefined:
            if (interpreter->pass == PassRun)
            {
                interpreter->state = StateEnd;
            }
            break;
            
        case TokenREM:
            ++interpreter->pc;
            break;
            
        case TokenLabel:
            ++interpreter->pc;
            if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
            ++interpreter->pc;
            break;
        
        case TokenEol:
            interpreter->isSingleLineIf = false;
            ++interpreter->pc;
            break;
            
        case TokenEND:
            if (itp_getNextTokenType(interpreter) == TokenIF)
            {
                return cmd_ENDIF(core);
            }
            return cmd_END(core);
            
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
            return cmd_IF(core);
        
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
            return cmd_POKE(core);
            
        case TokenFILL:
            return cmd_FILL(core);
            
        case TokenCOPY:
            return cmd_COPY(core);
            
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

        case TokenRANDOMIZE:
            return cmd_RANDOMIZE(core);
            
        case TokenLEFT:
        case TokenRIGHT:
            return cmd_LEFT_RIGHT(core);
            
        case TokenMID:
            return cmd_MID(core);
            
        default:
            printf("Command not implemented: %s\n", TokenStrings[interpreter->pc->type]);
            return ErrorUnexpectedToken;
    }
    return ErrorNone;
}
