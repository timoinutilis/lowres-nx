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
#include "lowres_core.h"
#include "text_lib.h"

enum ErrorCode cmd_PRINT(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    bool newLine = true;
    
    // PRINT
    ++interpreter->pc;
    
    while (!LRC_isEndOfCommand(interpreter))
    {
        struct TypedValue value = LRC_evaluateExpression(core, TypeClassAny);
        if (value.type == ValueError) return value.v.errorCode;
        
        if (interpreter->pass == PassRun)
        {
            if (value.type == ValueString)
            {
                LRC_printText(core, value.v.stringValue->chars);
                rcstring_release(value.v.stringValue);
            }
            else if (value.type == ValueFloat)
            {
                char buffer[20];
                snprintf(buffer, 20, "%d", (int)value.v.floatValue);
                LRC_printText(core, buffer);
            }
        }
        
        if (interpreter->pc->type == TokenComma)
        {
            if (interpreter->pass == PassRun)
            {
                LRC_printText(core, " ");
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
        LRC_printText(core, "\n");
    }
    return LRC_endOfCommand(interpreter);
}

enum ErrorCode cmd_TEXT(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // TEXT
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;

    // y value
    struct TypedValue yValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueError) return yValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;

    // string value
    struct TypedValue stringValue = LRC_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueError) return stringValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        LRC_writeText(core, stringValue.v.stringValue->chars, xValue.v.floatValue, yValue.v.floatValue);
    }
    
    return LRC_endOfCommand(interpreter);
}

enum ErrorCode cmd_NUMBER(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // NUMBER
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueError) return yValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // number value
    struct TypedValue numberValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (numberValue.type == ValueError) return numberValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // digits value
    struct TypedValue digitsValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (digitsValue.type == ValueError) return digitsValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        LRC_writeNumber(core, numberValue.v.floatValue, digitsValue.v.floatValue, xValue.v.floatValue, yValue.v.floatValue);
    }
    
    return LRC_endOfCommand(interpreter);
}
