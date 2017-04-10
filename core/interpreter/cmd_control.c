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
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // END
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->state = StateEnd;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_IF(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // IF
    struct Token *tokenIF = interpreter->pc;
    ++interpreter->pc;
    
    // Expression
    struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
    if (value.type == ValueTypeError) return value.v.errorCode;
    if (value.type != ValueTypeFloat) return ErrorTypeMismatch;
    
    // THEN
    if (interpreter->pc->type != TokenTHEN) return ErrorExpectedThen;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->pc->type == TokenEol)
        {
            // IF block
            if (interpreter->isSingleLineIf) return ErrorExpectedCommand;
            enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeIF, tokenIF);
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
    struct Interpreter *interpreter = &core->interpreter;
    
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
            if (!item || item->type != LabelTypeIF) return ErrorElseWithoutIf;
            item->token->jumpToken = interpreter->pc;
        
            enum ErrorCode errorCode = lab_pushLabelStackItem(interpreter, LabelTypeELSE, tokenELSE);
            if (errorCode != ErrorNone) return errorCode;
            
            if (interpreter->pc->type != TokenIF)
            {
                // Eol
                if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
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

enum ErrorCode cmd_ENDIF(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // END IF
    ++interpreter->pc;
    ++interpreter->pc;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct LabelStackItem *item = lab_popLabelStackItem(interpreter);
        if (!item)
        {
            return ErrorEndIfWithoutIf;
        }
        else if (item->type == LabelTypeIF)
        {
            item->token->jumpToken = interpreter->pc;
        }
        else if (item->type == LabelTypeELSE)
        {
            while (1)
            {
                item->token->jumpToken = interpreter->pc;
                
                item = lab_peekLabelStackItem(interpreter);
                if (item && item->type == LabelTypeELSE)
                {
                    item = lab_popLabelStackItem(interpreter);
                }
                else
                {
                    break;
                }
            }
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // FOR
    struct Token *tokenFOR = interpreter->pc;
    ++interpreter->pc;
    
    // Variable
    struct Token *tokenFORVar = interpreter->pc;
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;
    
    // Eq
    if (interpreter->pc->type != TokenEq) return ErrorExpectedEqualSign;
    ++interpreter->pc;
    
    // start value
    struct TypedValue startValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (startValue.type == ValueTypeError) return startValue.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
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
    if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
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
    struct Interpreter *interpreter = &core->interpreter;
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
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;
    
    if (interpreter->pass == PassPrepare)
    {
        if (tokenVar->symbolIndex != itemFORVar->token->symbolIndex) return ErrorNextWithoutFor;
    }
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
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
        if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
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
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // GOTO
    struct Token *tokenGOTO = interpreter->pc;
    ++interpreter->pc;
    
    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedLabel;
    struct Token *tokenIdentifier = interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        struct JumpLabelItem *item = lab_getJumpLabel(interpreter, tokenIdentifier->symbolIndex);
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // GOSUB
    struct Token *tokenGOSUB = interpreter->pc;
    ++interpreter->pc;
    
    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedLabel;
    struct Token *tokenIdentifier = interpreter->pc;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct JumpLabelItem *item = lab_getJumpLabel(interpreter, tokenIdentifier->symbolIndex);
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // RETURN
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        struct LabelStackItem *itemGOSUB = lab_popLabelStackItem(interpreter);
        if (!itemGOSUB) return ErrorReturnWithoutGosub;
        
        if (itemGOSUB->type == LabelTypeONGOSUB)
        {
            // exit from interrupt
            interpreter->exitEvaluation = true;
        }
        else if (itemGOSUB->type == LabelTypeGOSUB)
        {
            // jump back
            interpreter->pc = itemGOSUB->token; // after GOSUB
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
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // WAIT
    ++interpreter->pc;
    
    if (interpreter->pc->type == TokenVBL)
    {
        // VBL
        ++interpreter->pc;
    }
    else
    {
        // value
        struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
        if (value.type == ValueTypeError) return value.v.errorCode;
        
        if (interpreter->pass == PassRun)
        {
            interpreter->state = StateWait;
            interpreter->waitCount = value.v.floatValue;
        }
    }
    
    if (interpreter->pass == PassRun)
    {
        interpreter->exitEvaluation = true;
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ON(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // ON
    ++interpreter->pc;
    
    // RASTER
    if (interpreter->pc->type != TokenRASTER) return ErrorUnexpectedToken;
    ++interpreter->pc;
    
    // GOSUB
    if (interpreter->pc->type != TokenGOSUB) return ErrorUnexpectedToken;
    struct Token *tokenGOSUB = interpreter->pc;
    ++interpreter->pc;
    
    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedLabel;
    struct Token *tokenIdentifier = interpreter->pc;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        struct JumpLabelItem *item = lab_getJumpLabel(interpreter, tokenIdentifier->symbolIndex);
        if (!item) return ErrorUndefinedLabel;
        tokenGOSUB->jumpToken = item->token;
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->currentOnRasterToken = tokenGOSUB->jumpToken; // after label
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_DO(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // DO
    ++interpreter->pc;

    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // LOOP
    struct Token *tokenLOOP = interpreter->pc;
    ++interpreter->pc;
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
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

enum ErrorCode cmd_EXIT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // EXIT
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        
    }
    
    return itp_endOfCommand(interpreter);
}
