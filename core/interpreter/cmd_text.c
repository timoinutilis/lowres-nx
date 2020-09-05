//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "cmd_text.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "core.h"
#include "text_lib.h"

enum ErrorCode cmd_PRINT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    struct TextLib *lib = &interpreter->textLib;
    
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
                txtlib_printText(lib, value.v.stringValue->chars);
            }
            else if (value.type == ValueTypeFloat)
            {
                char buffer[20];
                snprintf(buffer, 20, "%0.7g", value.v.floatValue);
                txtlib_printText(lib, buffer);
            }
        }
        
        if (interpreter->pc->type == TokenComma)
        {
            if (interpreter->pass == PassRun)
            {
                txtlib_printText(lib, " ");
            }
            ++interpreter->pc;
            newLine = false;
        }
        else if (interpreter->pc->type == TokenSemicolon)
        {
            ++interpreter->pc;
            newLine = false;
        }
        else if (itp_isEndOfCommand(interpreter))
        {
            newLine = true;
        }
        else
        {
            return ErrorSyntax;
        }
    }
    
    if (interpreter->pass == PassRun && newLine)
    {
        txtlib_printText(lib, "\n");
    }
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_INPUT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    struct TextLib *lib = &interpreter->textLib;
    
    // INPUT
    ++interpreter->pc;
    
    if (interpreter->pc->type == TokenString)
    {
        // prompt
        if (interpreter->pass == PassRun)
        {
            txtlib_printText(lib, interpreter->pc->stringValue->chars);
        }
        ++interpreter->pc;
        
        // semicolon
        if (interpreter->pc->type != TokenSemicolon) return ErrorSyntax;
        ++interpreter->pc;
    }
    
    if (interpreter->pass == PassRun)
    {
        txtlib_inputBegin(lib);
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
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
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
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // y value
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;

    // string value
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        struct TextLib *lib = &interpreter->textLib;
        txtlib_writeText(lib, stringValue.v.stringValue->chars, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_NUMBER(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // NUMBER
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // number value
    struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numberValue.type == ValueTypeError) return numberValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // digits value
    struct TypedValue digitsValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (digitsValue.type == ValueTypeError) return digitsValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int digits = digitsValue.v.floatValue;
        struct TextLib *lib = &interpreter->textLib;
        txtlib_writeNumber(lib, numberValue.v.floatValue, digits, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_CLS(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    struct TextLib *lib = &interpreter->textLib;
    
    // CLS
    ++interpreter->pc;
    
    if (itp_isEndOfCommand(interpreter))
    {
        if (interpreter->pass == PassRun)
        {
            // clear all
            txtlib_clearScreen(lib);
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
            txtlib_clearBackground(lib, bgValue.v.floatValue);
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
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // w value
    struct TypedValue wValue = itp_evaluateNumericExpression(core, 1, PLANE_COLUMNS);
    if (wValue.type == ValueTypeError) return wValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // h value
    struct TypedValue hValue = itp_evaluateNumericExpression(core, 1, PLANE_ROWS);
    if (hValue.type == ValueTypeError) return hValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
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
        core->interpreter->textLib.cursorX = 0;
        core->interpreter->textLib.cursorY = 0;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FONT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // FONT
    ++interpreter->pc;
    
    // char value
    struct TypedValue cValue = itp_evaluateNumericExpression(core, 0, NUM_CHARACTERS - 1);
    if (cValue.type == ValueTypeError) return cValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->textLib.fontCharOffset = cValue.v.floatValue;
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
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
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

struct TypedValue fnc_CURSOR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // CURSOR.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
        
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenCURSORX:
                value.v.floatValue = interpreter->textLib.cursorX;
                break;
                
            case TokenCURSORY:
                value.v.floatValue = interpreter->textLib.cursorY;
                break;
                
            default:
                assert(0);
                break;
        }
    }
    return value;
}

enum ErrorCode cmd_CLW(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // CLW
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_clearWindow(&interpreter->textLib);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_TRACE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    struct TextLib *lib = &core->overlay->textLib;
    bool debug = interpreter->debug;
    
    do
    {
        // TRACE or comma
        bool separate = (interpreter->pc->type == TokenComma);
        ++interpreter->pc;
        
        struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
        if (value.type == ValueTypeError) return value.v.errorCode;
        
        if (interpreter->pass == PassRun)
        {
            if (separate && debug)
            {
                txtlib_printText(lib, " ");
            }
            if (value.type == ValueTypeString)
            {
                if (debug)
                {
                    txtlib_printText(lib, value.v.stringValue->chars);
                }
                rcstring_release(value.v.stringValue);
            }
            else if (value.type == ValueTypeFloat)
            {
                if (debug)
                {
                    char buffer[20];
                    snprintf(buffer, 20, "%0.7g", value.v.floatValue);
                    txtlib_printText(lib, buffer);
                }
            }
        }
    }
    while (interpreter->pc->type == TokenComma);
    
    if (interpreter->pass == PassRun && debug)
    {
        txtlib_printText(lib, "\n");
    }
    
    return itp_endOfCommand(interpreter);
}

