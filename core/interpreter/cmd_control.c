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

enum ErrorCode cmd_END(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    ++interpreter->pc;
    if (interpreter->pass == PASS_RUN)
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
    struct TypedValue value = LRC_evaluateExpression(core);
    if (value.type == ValueError) return value.v.errorCode;
    if (value.type != ValueFloat) return ErrorTypeMismatch;
    
    // THEN
    if (interpreter->pc->type != TokenTHEN) return ErrorExpectedThen;
    ++interpreter->pc;
    
    if (interpreter->pass == PASS_PREPARE)
    {
        if (interpreter->pc->type == TokenEol)
        {
            // IF block
            if (interpreter->isSingleLineIf) return ErrorExpectedCommand;
            LRC_pushLabelStackItem(interpreter, TokenIF, tokenIF);
            
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
    else if (interpreter->pass == PASS_RUN)
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
    
    if (interpreter->pass == PASS_PREPARE)
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
            if (!item || item->type != TokenIF) return ErrorElseWithoutIf;
            item->token->jumpToken = interpreter->pc;
        
            LRC_pushLabelStackItem(interpreter, TokenELSE, tokenELSE);
            
            if (interpreter->pc->type != TokenIF)
            {
                // Eol
                if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
                ++interpreter->pc;
            }
        }
    }
    else if (interpreter->pass == PASS_RUN)
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
    
    if (interpreter->pass == PASS_PREPARE)
    {
        struct LabelStackItem *item = LRC_popLabelStackItem(interpreter);
        if (!item)
        {
            return ErrorEndIfWithoutIf;
        }
        else if (item->type == TokenIF)
        {
            item->token->jumpToken = interpreter->pc;
        }
        else if (item->type == TokenELSE)
        {
            while (1)
            {
                item->token->jumpToken = interpreter->pc;
                
                item = LRC_peekLabelStackItem(interpreter);
                if (item && item->type == TokenELSE)
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
