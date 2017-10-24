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

#include "cmd_text.h"
#include <stdbool.h>
#include <stdlib.h>
#include "core.h"
#include "text_lib.h"
#include "interpreter_utils.h"

enum ErrorCode cmd_PRINT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
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
                snprintf(buffer, 20, "%0.7g", value.v.floatValue);
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
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
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
    struct Interpreter *interpreter = core->interpreter;
    
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
            varValue->floatValue = atof(interpreter->textLib.inputBuffer);
        }
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_TEXT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // TEXT
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;

    // y value
    struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
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
    struct Interpreter *interpreter = core->interpreter;
    
    // NUMBER
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
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
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // CLS
    ++interpreter->pc;
    
    if (itp_isEndOfCommand(interpreter))
    {
        if (interpreter->pass == PassRun)
        {
            // clear all
            txtlib_clearScreen(core);
        }
    }
    else
    {
        // bg value
        struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 1);
        if (bgValue.type == ValueTypeError) return bgValue.v.errorCode;
        
        if (interpreter->pass == PassRun)
        {
            // clear bg
            txtlib_clearBackground(core, bgValue.v.floatValue);
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_WINDOW(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // WINDOW
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // w value
    struct TypedValue wValue = itp_evaluateNumericExpression(core, 1, PLANE_COLUMNS);
    if (wValue.type == ValueTypeError) return wValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // h value
    struct TypedValue hValue = itp_evaluateNumericExpression(core, 1, PLANE_ROWS);
    if (hValue.type == ValueTypeError) return hValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // bg value
    struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 1);
    if (bgValue.type == ValueTypeError) return bgValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        core->interpreter->textLib.windowX = xValue.v.floatValue;
        core->interpreter->textLib.windowY = yValue.v.floatValue;
        core->interpreter->textLib.windowWidth = wValue.v.floatValue;
        core->interpreter->textLib.windowHeight = hValue.v.floatValue;
        core->interpreter->textLib.bg = bgValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FONT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // FONT
    ++interpreter->pc;
    
    // char value
    struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
    if (cValue.type == ValueTypeError) return cValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // attributes value
    struct TypedValue aValue = itp_evaluateCharAttributes(core, interpreter->textLib.fontCharAttr, true);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        if (cValue.type != ValueTypeNull) interpreter->textLib.fontCharOffset = cValue.v.floatValue;
        if (aValue.type != ValueTypeNull) interpreter->textLib.fontCharAttr.value = aValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_LOCATE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // LOCATE
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, core->interpreter->textLib.windowWidth - 1);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, core->interpreter->textLib.windowHeight - 1);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        core->interpreter->textLib.cursorX = xValue.v.floatValue;
        core->interpreter->textLib.cursorY = yValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_CLW(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // CLW
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_clearWindow(core);
    }
    
    return itp_endOfCommand(interpreter);
}
