//
// Copyright 2017 Timo Kloss
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

#include "cmd_control.h"
#include "core.h"
#include <assert.h>

enum ErrorCode cmd_END(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // END
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        itp_endProgram(core);
        return ErrorNone;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_IF(struct Core *core, bool isAfterBlockElse)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // IF
    struct Token *tokenIF = interpreter->pc;
    ++interpreter->pc;
    
    // Expression
    struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    // THEN
    if (interpreter->pc->type != TokenTHEN) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->pc->type == TokenEol)
        {
            // IF block
            if (interpreter->isSingleLineIf) return ErrorExpectedCommand;
            enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, isAfterBlockElse ? LabelTypeELSEIF : LabelTypeIF, tokenIF);
            if (errorCode != ErrorNone) return errorCode;
            
            // Eol
            ++interpreter->pc;
        }
        else
        {
            // single line IF
            interpreter->isSingleLineIf = true;
            struct Token *token = interpreter->pc;
            while (token->type != TokenEol && token->type != TokenELSE)
            {
                token++;
            }
            tokenIF->jumpToken = token + 1; // after ELSE or Eol
        }
    }
    else if (interpreter->pass == PassRun)
    {
        if (value.v.floatValue == 0)
        {
            interpreter->pc = tokenIF->jumpToken; // after ELSE or END IF, or Eol for single line
        }
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_ELSE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ELSE
    struct Token *tokenELSE = interpreter->pc;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->isSingleLineIf)
        {
            if (interpreter->pc->type == TokenEol) return ErrorExpectedCommand;
            struct Token *token = interpreter->pc;
            while (token->type != TokenEol)
            {
                token++;
            }
            tokenELSE->jumpToken = token + 1; // after Eol
        }
        else
        {
            struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
            if (!item) return ErrorElseWithoutIf;
            if (item->type == LabelTypeIF)
            {
                item->token->jumpToken = interpreter->pc;
            }
            else if (item->type == LabelTypeELSEIF)
            {
                item->token->jumpToken = interpreter->pc;
                
                item = lab_popLabelStackItem(interpreter);
                assert(item->type == LabelTypeELSE);
                item->token->jumpToken = tokenELSE;
            }
            else
            {
                return ErrorElseWithoutIf;
            }
            
            enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeELSE, tokenELSE);
            if (errorCode != ErrorNone) return errorCode;
            
            if (interpreter->pc->type == TokenIF)
            {
                return cmd_IF(core, true);
            }
            else
            {
                // Eol
                if (interpreter->pc->type != TokenEol) return ErrorSyntax;
                ++interpreter->pc;
            }
        }
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->pc = tokenELSE->jumpToken; // after END IF, or Eol for single line
    }
    return ErrorNone;
}

enum ErrorCode cmd_END_IF(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // END IF
    ++interpreter->pc;
    ++interpreter->pc;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
        if (!item)
        {
            return ErrorEndIfWithoutIf;
        }
        else if (item->type == LabelTypeIF || item->type == LabelTypeELSE)
        {
            item->token->jumpToken = interpreter->pc;
        }
        else if (item->type == LabelTypeELSEIF)
        {
            item->token->jumpToken = interpreter->pc;
            
            item = lab_popLabelStackItem(interpreter);
            assert(item->type == LabelTypeELSE);
            item->token->jumpToken = interpreter->pc;
        }
        else
        {
            return ErrorEndIfWithoutIf;
        }
    }
    return ErrorNone;
}

enum ErrorCode cmd_FOR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // FOR
    struct Token *tokenFOR = interpreter->pc;
    ++interpreter->pc;
    
    // Variable
    struct Token *tokenFORVar = interpreter->pc;
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;
    
    // Eq
    if (interpreter->pc->type != TokenEq) return ErrorSyntax;
    ++interpreter->pc;
    
    // start value
    struct TypedValue startValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (startValue.type == ValueTypeError) return startValue.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
    ++interpreter->pc;

    // limit value
    struct Token *tokenFORLimit = interpreter->pc;
    struct TypedValue limitValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (limitValue.type == ValueTypeError) return limitValue.v.errorCode;
    
    // STEP
    struct TypedValue stepValue;
    if (interpreter->pc->type == TokenSTEP)
    {
        ++interpreter->pc;
        
        // step value
        stepValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (stepValue.type == ValueTypeError) return stepValue.v.errorCode;
    }
    else
    {
        stepValue.type = ValueTypeFloat;
        stepValue.v.floatValue = 1.0f;
    }
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        lab_pushLabelStackItem(interpreter, LabelTypeFORLimit, tokenFORLimit);
        lab_pushLabelStackItem(interpreter, LabelTypeFORVar, tokenFORVar);
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeFOR, tokenFOR);
        if (errorCode != ErrorNone) return errorCode;
    }
    else if (interpreter->pass == PassRun)
    {
        varValue->floatValue = startValue.v.floatValue;
        
        // limit check
        if ((stepValue.v.floatValue > 0 && varValue->floatValue > limitValue.v.floatValue) || (stepValue.v.floatValue < 0 && varValue->floatValue < limitValue.v.floatValue))
        {
            interpreter->pc = tokenFOR->jumpToken; // after NEXT's Eol
        }
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_NEXT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    struct LabelStackItem *itemFORLimit = NULL;
    struct LabelStackItem *itemFORVar = NULL;
    struct LabelStackItem *itemFOR = NULL;
    
    if (interpreter->pass == PassPrepare)
    {
        itemFOR = lab_popLabelStackItem(interpreter);
        if (!itemFOR || itemFOR->type != LabelTypeFOR) return ErrorNextWithoutFor;
        
        itemFORVar = lab_popLabelStackItem(interpreter);
        assert(itemFORVar && itemFORVar->type == LabelTypeFORVar);
        
        itemFORLimit = lab_popLabelStackItem(interpreter);
        assert(itemFORLimit && itemFORLimit->type == LabelTypeFORLimit);
    }
    
    // NEXT
    struct Token *tokenNEXT = interpreter->pc;
    ++interpreter->pc;
    
    // Variable
    enum ErrorCode errorCode = ErrorNone;
    struct Token *tokenVar = interpreter->pc;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;
    
    if (interpreter->pass == PassPrepare)
    {
        if (tokenVar->symbolIndex != itemFORVar->token->symbolIndex) return ErrorNextWithoutFor;
    }
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        itemFOR->token->jumpToken = interpreter->pc;
        tokenNEXT->jumpToken = itemFORLimit->token;
    }
    else if (interpreter->pass == PassRun)
    {
        struct Token *storedPc = interpreter->pc;
        interpreter->pc = tokenNEXT->jumpToken;
        
        // limit value
        struct TypedValue limitValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (limitValue.type == ValueTypeError) return limitValue.v.errorCode;
        
        // STEP
        struct TypedValue stepValue;
        if (interpreter->pc->type == TokenSTEP)
        {
            ++interpreter->pc;
            
            // step value
            stepValue = itp_evaluateExpression(core, TypeClassNumeric);
            if (stepValue.type == ValueTypeError) return stepValue.v.errorCode;
        }
        else
        {
            stepValue.type = ValueTypeFloat;
            stepValue.v.floatValue = 1.0f;
        }
        
        // Eol
        if (interpreter->pc->type != TokenEol) return ErrorSyntax;
        ++interpreter->pc;

        varValue->floatValue += stepValue.v.floatValue;
        
        // limit check
        if ((stepValue.v.floatValue > 0 && varValue->floatValue > limitValue.v.floatValue) || (stepValue.v.floatValue < 0 && varValue->floatValue < limitValue.v.floatValue))
        {
            interpreter->pc = storedPc; // after NEXT's Eol
        }
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_GOTO(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // GOTO
    struct Token *tokenGOTO = interpreter->pc;
    ++interpreter->pc;
    
    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedLabel;
    struct Token *tokenIdentifier = interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
        if (!item) return ErrorUndefinedLabel;
        tokenGOTO->jumpToken = item->token;
        
        return itp_endOfCommand(interpreter);
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->pc = tokenGOTO->jumpToken; // after label
    }
    return ErrorNone;
}

enum ErrorCode cmd_GOSUB(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // GOSUB
    struct Token *tokenGOSUB = interpreter->pc;
    ++interpreter->pc;
    
    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedLabel;
    struct Token *tokenIdentifier = interpreter->pc;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
        if (!item) return ErrorUndefinedLabel;
        tokenGOSUB->jumpToken = item->token;
        
        return itp_endOfCommand(interpreter);
    }
    else if (interpreter->pass == PassRun)
    {
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeGOSUB, interpreter->pc);
        if (errorCode != ErrorNone) return errorCode;
        
        interpreter->pc = tokenGOSUB->jumpToken; // after label
    }
    return ErrorNone;
}

enum ErrorCode cmd_RETURN(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // RETURN
    struct Token *tokenRETURN = interpreter->pc;
    ++interpreter->pc;
    
    // Identifier
    struct Token *tokenIdentifier = NULL;
    if (interpreter->pc->type == TokenIdentifier)
    {
        tokenIdentifier = interpreter->pc;
        ++interpreter->pc;
    }
    
    if (interpreter->pass == PassPrepare)
    {
        if (tokenIdentifier)
        {
            struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
            if (!item) return ErrorUndefinedLabel;
            tokenRETURN->jumpToken = item->token;
        }
    }
    else if (interpreter->pass == PassRun)
    {
        struct LabelStackItem *itemGOSUB = lab_popLabelStackItem(interpreter);
        if (!itemGOSUB) return ErrorReturnWithoutGosub;
        
        if (itemGOSUB->type == LabelTypeGOSUB)
        {
            if (tokenRETURN->jumpToken)
            {
                // jump to label
                interpreter->pc = tokenRETURN->jumpToken; // after label
                // clear stack
                interpreter->numLabelStackItems = 0;
            }
            else
            {
                // jump back
                interpreter->pc = itemGOSUB->token; // after GOSUB
            }
        }
        else
        {
            return ErrorReturnWithoutGosub;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_WAIT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // WAIT
    ++interpreter->pc;
    
    int wait = 0;
    if (interpreter->pc->type == TokenVBL)
    {
        // VBL
        ++interpreter->pc;
    }
    else
    {
        // value
        struct TypedValue value = itp_evaluateNumericExpression(core, 1, 0xFFFF);
        if (value.type == ValueTypeError) return value.v.errorCode;
        wait = value.v.floatValue - 1;
    }
    
    if (interpreter->pass == PassRun)
    {
        interpreter->exitEvaluation = true;
        interpreter->waitCount = wait;
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ON(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ON
    ++interpreter->pc;
    
    // RASTER/VBL
    enum TokenType type = interpreter->pc->type;
    if (type != TokenRASTER && type != TokenVBL) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pc->type == TokenOFF)
    {
        // OFF
        ++interpreter->pc;
        
        if (interpreter->pass == PassRun)
        {
            if (type == TokenRASTER)
            {
                interpreter->currentOnRasterToken = NULL;
            }
            else if (type == TokenVBL)
            {
                interpreter->currentOnVBLToken = NULL;
            }
        }
    }
    else
    {
        // CALL
        if (interpreter->pc->type != TokenCALL) return ErrorSyntax;
        struct Token *tokenCALL = interpreter->pc;
        ++interpreter->pc;
        
        // Identifier
        if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedSubprogramName;
        struct Token *tokenIdentifier = interpreter->pc;
        ++interpreter->pc;
        
        if (interpreter->pass == PassPrepare)
        {
            struct SubItem *item = tok_getSub(&interpreter->tokenizer, tokenIdentifier->symbolIndex);
            if (!item) return ErrorUndefinedSubprogram;
            tokenCALL->jumpToken = item->token;
        }
        else if (interpreter->pass == PassRun)
        {
            if (type == TokenRASTER)
            {
                interpreter->currentOnRasterToken = tokenCALL->jumpToken;
            }
            else if (type == TokenVBL)
            {
                interpreter->currentOnVBLToken = tokenCALL->jumpToken;
            }
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_DO(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // DO
    ++interpreter->pc;

    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeDO, interpreter->pc);
        if (errorCode != ErrorNone) return errorCode;
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_LOOP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // LOOP
    struct Token *tokenLOOP = interpreter->pc;
    ++interpreter->pc;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
        if (!item || item->type != LabelTypeDO) return ErrorLoopWithoutDo;
        
        tokenLOOP->jumpToken = item->token;
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->pc = tokenLOOP->jumpToken; // after DO
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_REPEAT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // REPEAT
    ++interpreter->pc;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeREPEAT, interpreter->pc);
        if (errorCode != ErrorNone) return errorCode;
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_UNTIL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // UNTIL
    struct Token *tokenUNTIL = interpreter->pc;
    ++interpreter->pc;
    
    // Expression
    struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
        if (!item || item->type != LabelTypeREPEAT) return ErrorUntilWithoutRepeat;
        
        tokenUNTIL->jumpToken = item->token;
    }
    else if (interpreter->pass == PassRun)
    {
        if (value.v.floatValue == 0)
        {
            interpreter->pc = tokenUNTIL->jumpToken; // after REPEAT
        }
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_WHILE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // WHILE
    struct Token *tokenWHILE = interpreter->pc;
    if (interpreter->pass == PassPrepare)
    {
        enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeWHILE, tokenWHILE);
        if (errorCode != ErrorNone) return errorCode;
    }
    ++interpreter->pc;
    
    // Expression
    struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        if (value.v.floatValue == 0)
        {
            interpreter->pc = tokenWHILE->jumpToken; // after WEND
        }
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_WEND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // WEND
    struct Token *tokenWEND = interpreter->pc;
    ++interpreter->pc;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
        if (!item || item->type != LabelTypeWHILE) return ErrorWendWithoutWhile;
        
        tokenWEND->jumpToken = item->token;
        item->token->jumpToken = interpreter->pc;
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->pc = tokenWEND->jumpToken; // on WHILE
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_SYSTEM(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SYSTEM
    ++interpreter->pc;
    
    // type value
    struct TypedValue tValue = itp_evaluateNumericExpression(core, 0, 0);
    if (tValue.type == ValueTypeError) return tValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // setting value
    struct TypedValue sValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (sValue.type == ValueTypeError) return sValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        switch ((int)tValue.v.floatValue)
        {
            case 0:
                core->machineInternals->isEnergySaving = (sValue.v.floatValue != 0.0f);
                break;
        }
    }
    
    return itp_endOfCommand(interpreter);

}
