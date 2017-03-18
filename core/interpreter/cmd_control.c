//
// Copyright 2017 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#include "cmd_control.h"
#include "lowres_core.h"
#include <assert.h>

enum ErrorCode cmd_END(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    ++interpreter->pc;
    if (interpreter->pass == PassRun)
    {
        return LRC_endOfCommand(interpreter) ?: ErrorEndOfProgram;
    }
    return LRC_endOfCommand(interpreter);
}

enum ErrorCode cmd_IF(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // IF
    struct Token *tokenIF = interpreter->pc;
    ++interpreter->pc;
    
    // Expression
    struct TypedValue value = LRC_evaluateExpression(core, TypeClassNumeric);
    if (value.type == ValueError) return value.v.errorCode;
    if (value.type != ValueFloat) return ErrorTypeMismatch;
    
    // THEN
    if (interpreter->pc->type != TokenTHEN) return ErrorExpectedThen;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        if (interpreter->pc->type == TokenEol)
        {
            // IF block
            if (interpreter->isSingleLineIf) return ErrorExpectedCommand;
            LRC_pushLabelStackItem(interpreter, LabelTypeIF, tokenIF);
            
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

enum ErrorCode cmd_ELSE(struct LowResCore *core)
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
            struct LabelStackItem *item = LRC_popLabelStackItem(interpreter);
            if (!item || item->type != LabelTypeIF) return ErrorElseWithoutIf;
            item->token->jumpToken = interpreter->pc;
        
            LRC_pushLabelStackItem(interpreter, LabelTypeELSE, tokenELSE);
            
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

enum ErrorCode cmd_ENDIF(struct LowResCore *core)
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
        struct LabelStackItem *item = LRC_popLabelStackItem(interpreter);
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
                
                item = LRC_peekLabelStackItem(interpreter);
                if (item && item->type == LabelTypeELSE)
                {
                    item = LRC_popLabelStackItem(interpreter);
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

enum ErrorCode cmd_FOR(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // FOR
    struct Token *tokenFOR = interpreter->pc;
    ++interpreter->pc;
    
    // Variable
    struct Token *tokenFORVar = interpreter->pc;
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueNull;
    union Value *varValue = LRC_readVariable(core, &valueType, &errorCode);
    if (!varValue) return errorCode;
    if (valueType != ValueFloat) return ErrorTypeMismatch;
    
    // Eq
    if (interpreter->pc->type != TokenEq) return ErrorExpectedEqualSign;
    ++interpreter->pc;
    
    // start value
    struct TypedValue startValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (startValue.type == ValueError) return startValue.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
    ++interpreter->pc;

    // limit value
    struct Token *tokenFORLimit = interpreter->pc;
    struct TypedValue limitValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (limitValue.type == ValueError) return limitValue.v.errorCode;
    
    // STEP
    struct TypedValue stepValue;
    if (interpreter->pc->type == TokenSTEP)
    {
        ++interpreter->pc;
        
        // step value
        stepValue = LRC_evaluateExpression(core, TypeClassNumeric);
        if (stepValue.type == ValueError) return stepValue.v.errorCode;
    }
    else
    {
        stepValue.type = ValueFloat;
        stepValue.v.floatValue = 1.0f;
    }
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
    ++interpreter->pc;
    
    if (interpreter->pass == PassPrepare)
    {
        LRC_pushLabelStackItem(interpreter, LabelTypeFORLimit, tokenFORLimit);
        LRC_pushLabelStackItem(interpreter, LabelTypeFORVar, tokenFORVar);
        LRC_pushLabelStackItem(interpreter, LabelTypeFOR, tokenFOR);
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

enum ErrorCode cmd_NEXT(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    struct LabelStackItem *itemFORLimit = NULL;
    struct LabelStackItem *itemFORVar = NULL;
    struct LabelStackItem *itemFOR = NULL;
    
    if (interpreter->pass == PassPrepare)
    {
        itemFOR = LRC_popLabelStackItem(interpreter);
        if (!itemFOR || itemFOR->type != LabelTypeFOR) return ErrorNextWithoutFor;
        
        itemFORVar = LRC_popLabelStackItem(interpreter);
        assert(itemFORVar && itemFORVar->type == LabelTypeFORVar);
        
        itemFORLimit = LRC_popLabelStackItem(interpreter);
        assert(itemFORLimit && itemFORLimit->type == LabelTypeFORLimit);
    }
    
    // NEXT
    struct Token *tokenNEXT = interpreter->pc;
    ++interpreter->pc;
    
    // Variable
    enum ErrorCode errorCode = ErrorNone;
    struct Token *tokenVar = interpreter->pc;
    enum ValueType valueType = ValueNull;
    union Value *varValue = LRC_readVariable(core, &valueType, &errorCode);
    if (!varValue) return errorCode;
    if (valueType != ValueFloat) return ErrorTypeMismatch;
    
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
        struct TypedValue limitValue = LRC_evaluateExpression(core, TypeClassNumeric);
        if (limitValue.type == ValueError) return limitValue.v.errorCode;
        
        // STEP
        struct TypedValue stepValue;
        if (interpreter->pc->type == TokenSTEP)
        {
            ++interpreter->pc;
            
            // step value
            stepValue = LRC_evaluateExpression(core, TypeClassNumeric);
            if (stepValue.type == ValueError) return stepValue.v.errorCode;
        }
        else
        {
            stepValue.type = ValueFloat;
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

enum ErrorCode cmd_GOTO(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // GOTO
    struct Token *tokenGOTO = interpreter->pc;
    ++interpreter->pc;
    
    // Identifier
    if (interpreter->pc->type != TokenIdentifier) return ErrorExpectedLabel;
    struct Token *tokenIdentifier = interpreter->pc;
    ++interpreter->pc;

    if (interpreter->pass == PassPrepare)
    {
        struct JumpLabelItem *item = LRC_getJumpLabel(interpreter, tokenIdentifier->symbolIndex);
        if (!item) return ErrorUndefinedLabel;
        tokenGOTO->jumpToken = item->token;
    }
    else if (interpreter->pass == PassRun)
    {
        interpreter->pc = tokenGOTO->jumpToken; // after label
    }

    return ErrorNone;
}
