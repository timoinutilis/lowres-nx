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

#include "cmd_background.h"
#include "core.h"
#include "text_lib.h"
#include "cmd_text.h"
#include "interpreter_utils.h"
#include <assert.h>

enum ErrorCode cmd_BG(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BG
    ++interpreter->pc;
    
    // bg value
    struct TypedValue bgValue = itp_evaluateNumericExpression(core, 0, 1);
    if (bgValue.type == ValueTypeError) return bgValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->textLib.bg = bgValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_SOURCE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BG SOURCE
    ++interpreter->pc;
    ++interpreter->pc;
    
    // address value
    struct TypedValue aValue = itp_evaluateNumericExpression(core, 0, 0xFFFF);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;
    
    int w = 0;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // width value
        struct TypedValue wValue = itp_evaluateNumericExpression(core, 1, 0xFFFF);
        if (wValue.type == ValueTypeError) return wValue.v.errorCode;
        
        w = wValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        int address = aValue.v.floatValue;
        if (w > 0)
        {
            core->interpreter->textLib.sourceAddress = address;
            core->interpreter->textLib.sourceWidth = w;
        }
        else
        {
            // data with preceding size (W x H)
            core->interpreter->textLib.sourceAddress = address + 2;
            core->interpreter->textLib.sourceWidth = machine_peek(core, address);
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_COPY(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BG COPY
    ++interpreter->pc;
    ++interpreter->pc;
    
    // src X value
    struct TypedValue srcXValue = itp_evaluateNumericExpression(core, 0, 0xFFFF);
    if (srcXValue.type == ValueTypeError) return srcXValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // src Y value
    struct TypedValue srcYValue = itp_evaluateNumericExpression(core, 0, 0xFFFF);
    if (srcYValue.type == ValueTypeError) return srcYValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // width value
    struct TypedValue wValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS);
    if (wValue.type == ValueTypeError) return wValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // height value
    struct TypedValue hValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS);
    if (hValue.type == ValueTypeError) return hValue.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
    ++interpreter->pc;
    
    // dst X value
    struct TypedValue dstXValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dstXValue.type == ValueTypeError) return dstXValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // dst Y value
    struct TypedValue dstYValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dstYValue.type == ValueTypeError) return dstYValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int w = wValue.v.floatValue;
        int h = hValue.v.floatValue;
        txtlib_copyBackground(core, srcXValue.v.floatValue, srcYValue.v.floatValue, w, h, dstXValue.v.floatValue, dstYValue.v.floatValue);
        interpreter->cycles += w * h * 2;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_SCROLL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BG SCROLL
    ++interpreter->pc;
    ++interpreter->pc;
    
    // x1 value
    struct TypedValue x1Value = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (x1Value.type == ValueTypeError) return x1Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y1 value
    struct TypedValue y1Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (y1Value.type == ValueTypeError) return y1Value.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
    ++interpreter->pc;
    
    // x2 value
    struct TypedValue x2Value = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (x2Value.type == ValueTypeError) return x2Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y2 value
    struct TypedValue y2Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (y2Value.type == ValueTypeError) return y2Value.v.errorCode;

    // STEP
    if (interpreter->pc->type != TokenSTEP) return ErrorUnexpectedToken;
    ++interpreter->pc;

    // dx value
    struct TypedValue dxValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dxValue.type == ValueTypeError) return dxValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // dy value
    struct TypedValue dyValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dyValue.type == ValueTypeError) return dyValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int x1 = x1Value.v.floatValue;
        int y1 = y1Value.v.floatValue;
        int x2 = x2Value.v.floatValue;
        int y2 = y2Value.v.floatValue;
        txtlib_scrollBackground(core, x1, y1, x2, y2, dxValue.v.floatValue, dyValue.v.floatValue);
        interpreter->cycles += (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ATTR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ATTR
    ++interpreter->pc;
    
    // attributes value
    struct TypedValue aValue = itp_evaluateCharAttributes(core, interpreter->textLib.charAttr, false);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;
    
    // filter value
    int filter = 0xFF;
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;
        
        struct TypedValue fValue = itp_evaluateNumericExpression(core, 0, 0xFF);
        if (fValue.type == ValueTypeError) return fValue.v.errorCode;
        filter = fValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        interpreter->textLib.charAttr.value = aValue.v.floatValue;
        interpreter->textLib.charAttrFilter = filter;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_FILL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BG FILL
    ++interpreter->pc;
    ++interpreter->pc;

    // x1 value
    struct TypedValue x1Value = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (x1Value.type == ValueTypeError) return x1Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y1 value
    struct TypedValue y1Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (y1Value.type == ValueTypeError) return y1Value.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
    ++interpreter->pc;

    // x2 value
    struct TypedValue x2Value = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (x2Value.type == ValueTypeError) return x2Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // y2 value
    struct TypedValue y2Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (y2Value.type == ValueTypeError) return y2Value.v.errorCode;

    // CHAR
    int character = -1;
    if (interpreter->pc->type == TokenCHAR)
    {
        ++interpreter->pc;
    
        // character value
        struct TypedValue cValue = itp_evaluateNumericExpression(core, 0, NUM_CHARACTERS - 1);
        if (cValue.type == ValueTypeError) return cValue.v.errorCode;
        character = cValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        int x1 = x1Value.v.floatValue;
        int y1 = y1Value.v.floatValue;
        int x2 = x2Value.v.floatValue;
        int y2 = y2Value.v.floatValue;
        txtlib_setCells(core, x1, y1, x2, y2, character);
        interpreter->cycles += (x2 - x1 + 1) * (y2 - y1 + 1) * 2;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_CELL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // CELL
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
    
    // character value
    int character = -1;
    struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
    if (cValue.type == ValueTypeError) return cValue.v.errorCode;
    if (cValue.type == ValueTypeFloat)
    {
        character = cValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        txtlib_setCell(core, xValue.v.floatValue, yValue.v.floatValue, character);
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_CELL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // CELL.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (xValue.type == ValueTypeError) return xValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (yValue.type == ValueTypeError) return yValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        struct Cell *cell = txtlib_getCell(core, xValue.v.floatValue, yValue.v.floatValue);
        if (type == TokenCELLA)
        {
            value.v.floatValue = cell->attr.value;
        }
        else if (type == TokenCELLC)
        {
            value.v.floatValue = cell->character;
        }
        else
        {
            assert(0);
        }
    }
    return value;
}
