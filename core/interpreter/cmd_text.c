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

#include "cmd_text.h"
#include <stdbool.h>
#include <stdlib.h>
#include "core.h"
#include "text_lib.h"

enum ErrorCode cmd_PRINT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    bool newLine = true;
    
    // PRINT
    ++interpreter->pc;
    
    while (!itp_isEndOfCommand(interpreter))
    {
        struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
        if (value.type == ValueTypeError) return value.v.errorCode;
        
        if (interpreter->pass == PassRun)
        {
            if (value.type == ValueTypeString)
            {
                txtlib_printText(core, value.v.stringValue->chars);
                rcstring_release(value.v.stringValue);
            }
            else if (value.type == ValueTypeFloat)
            {
                char buffer[20];
                snprintf(buffer, 20, "%d", (int)value.v.floatValue);
                txtlib_printText(core, buffer);
            }
        }
        
        if (interpreter->pc->type == TokenComma)
        {
            if (interpreter->pass == PassRun)
            {
                txtlib_printText(core, " ");
            }
            ++interpreter->pc;
            newLine = false;
        }
        else if (interpreter->pc->type == TokenSemicolon)
        {
            ++interpreter->pc;
            newLine = false;
        }
        else
        {
            newLine = true;
        }
    }
    
    if (interpreter->pass == PassRun && newLine)
    {
        txtlib_printText(core, "\n");
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_INPUT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // INPUT
    ++interpreter->pc;
    
    if (interpreter->pc->type == TokenString)
    {
        // prompt
        if (interpreter->pass == PassRun)
        {
            txtlib_printText(core, interpreter->pc->stringValue->chars);
        }
        ++interpreter->pc;
        
        // semicolon
        if (interpreter->pc->type != TokenSemicolon) return ErrorExpectedSemicolon;
        ++interpreter->pc;
    }
    
    if (interpreter->pass == PassRun)
    {
        txtlib_inputBegin(core);
        interpreter->state = StateInput;
    }
    else
    {
        return cmd_endINPUT(core);
    }
    
    return ErrorNone;
}

enum ErrorCode cmd_endINPUT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // identifier
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode);
    if (!varValue) return errorCode;
    
    if (interpreter->pass == PassRun)
    {
        if (valueType == ValueTypeString)
        {
            struct RCString *rcstring = rcstring_new(interpreter->textLib.inputBuffer, interpreter->textLib.inputLength);
            if (!rcstring) return ErrorOutOfMemory;
            
            if (varValue->stringValue)
            {
                rcstring_release(varValue->stringValue);
            }
            varValue->stringValue = rcstring;
        }
        else if (valueType == ValueTypeFloat)
        {
            varValue->floatValue = atoi(interpreter->textLib.inputBuffer);
        }
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_TEXT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // TEXT
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;

    // y value
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;

    // string value
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_writeText(core, stringValue.v.stringValue->chars, xValue.v.floatValue, yValue.v.floatValue);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_NUMBER(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // NUMBER
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // number value
    struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numberValue.type == ValueTypeError) return numberValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // digits value
    struct TypedValue digitsValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (digitsValue.type == ValueTypeError) return digitsValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_writeNumber(core, numberValue.v.floatValue, digitsValue.v.floatValue, xValue.v.floatValue, yValue.v.floatValue);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_CLS(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // CLS
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_clear(core);
    }
    
    return itp_endOfCommand(interpreter);
}
