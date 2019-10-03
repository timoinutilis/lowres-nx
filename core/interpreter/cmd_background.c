//
// Copyright 2017-2019 Timo Kloss
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
#include <math.h>

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
    
    int h = 0;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // height value
        struct TypedValue hValue = itp_evaluateNumericExpression(core, 1, 0xFFFF);
        if (hValue.type == ValueTypeError) return hValue.v.errorCode;
        
        h = hValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        int address = aValue.v.floatValue;
        if (w > 0)
        {
            core->interpreter->textLib.sourceAddress = address;
            core->interpreter->textLib.sourceWidth = w;
            core->interpreter->textLib.sourceHeight = h;
        }
        else
        {
            // data with preceding size (W x H)
            core->interpreter->textLib.sourceAddress = address + 4;
            core->interpreter->textLib.sourceWidth = machine_peek(core, address + 2);
            core->interpreter->textLib.sourceHeight = machine_peek(core, address + 3);
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
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // src Y value
    struct TypedValue srcYValue = itp_evaluateNumericExpression(core, 0, 0xFFFF);
    if (srcYValue.type == ValueTypeError) return srcYValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // width value
    struct TypedValue wValue = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS);
    if (wValue.type == ValueTypeError) return wValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // height value
    struct TypedValue hValue = itp_evaluateNumericExpression(core, 0, PLANE_ROWS);
    if (hValue.type == ValueTypeError) return hValue.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
    ++interpreter->pc;
    
    // dst X value
    struct TypedValue dstXValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dstXValue.type == ValueTypeError) return dstXValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // dst Y value
    struct TypedValue dstYValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dstYValue.type == ValueTypeError) return dstYValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_copyBackground(&interpreter->textLib, srcXValue.v.floatValue, srcYValue.v.floatValue, wValue.v.floatValue, hValue.v.floatValue, dstXValue.v.floatValue, dstYValue.v.floatValue);
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
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y1 value
    struct TypedValue y1Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (y1Value.type == ValueTypeError) return y1Value.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
    ++interpreter->pc;
    
    // x2 value
    struct TypedValue x2Value = itp_evaluateNumericExpression(core, 0, PLANE_COLUMNS - 1);
    if (x2Value.type == ValueTypeError) return x2Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y2 value
    struct TypedValue y2Value = itp_evaluateNumericExpression(core, 0, PLANE_ROWS - 1);
    if (y2Value.type == ValueTypeError) return y2Value.v.errorCode;

    // STEP
    if (interpreter->pc->type != TokenSTEP) return ErrorSyntax;
    ++interpreter->pc;

    // dx value
    struct TypedValue dxValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dxValue.type == ValueTypeError) return dxValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // dy value
    struct TypedValue dyValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (dyValue.type == ValueTypeError) return dyValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_scrollBackground(&interpreter->textLib, x1Value.v.floatValue, y1Value.v.floatValue, x2Value.v.floatValue, y2Value.v.floatValue, dxValue.v.floatValue, dyValue.v.floatValue);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ATTR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ATTR
    ++interpreter->pc;
    
    // attributes value
    struct TypedValue aValue = itp_evaluateCharAttributes(core, interpreter->textLib.charAttr);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->textLib.charAttr.value = aValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_PAL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // PAL
    ++interpreter->pc;
    
    // value
    struct TypedValue value = itp_evaluateNumericExpression(core, 0, NUM_PALETTES - 1);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->textLib.charAttr.palette = value.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FLIP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // FLIP
    ++interpreter->pc;
    
    // x value
    struct TypedValue fxValue = itp_evaluateNumericExpression(core, -1, 1);
    if (fxValue.type == ValueTypeError) return fxValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y value
    struct TypedValue fyValue = itp_evaluateNumericExpression(core, -1, 1);
    if (fyValue.type == ValueTypeError) return fyValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->textLib.charAttr.flipX = fxValue.v.floatValue ? 1 : 0;
        interpreter->textLib.charAttr.flipY = fyValue.v.floatValue ? 1 : 0;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_PRIO(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // PRIO
    ++interpreter->pc;
    
    // value
    struct TypedValue value = itp_evaluateNumericExpression(core, -1, 1);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->textLib.charAttr.priority = value.v.floatValue ? 1 : 0;
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
    struct TypedValue x1Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (x1Value.type == ValueTypeError) return x1Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y1 value
    struct TypedValue y1Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (y1Value.type == ValueTypeError) return y1Value.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
    ++interpreter->pc;

    // x2 value
    struct TypedValue x2Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (x2Value.type == ValueTypeError) return x2Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y2 value
    struct TypedValue y2Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (y2Value.type == ValueTypeError) return y2Value.v.errorCode;

    // CHAR
    if (interpreter->pc->type == TokenCHAR)
    {
        ++interpreter->pc;
        
        // write character with current attributes
    
        // character value
        struct TypedValue cValue = itp_evaluateNumericExpression(core, 0, NUM_CHARACTERS - 1);
        if (cValue.type == ValueTypeError) return cValue.v.errorCode;
        
        if (interpreter->pass == PassRun)
        {
            txtlib_setCells(&interpreter->textLib, floorf(x1Value.v.floatValue), floorf(y1Value.v.floatValue), floorf(x2Value.v.floatValue), floorf(y2Value.v.floatValue), cValue.v.floatValue);
        }
    }
    else
    {
        // write current attributes (obsolete syntax!)
        
        if (interpreter->pass == PassRun)
        {
            txtlib_setCells(&interpreter->textLib, floorf(x1Value.v.floatValue), floorf(y1Value.v.floatValue), floorf(x2Value.v.floatValue), floorf(y2Value.v.floatValue), -1);
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_BG_TINT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BG TINT
    ++interpreter->pc;
    ++interpreter->pc;
    
    // x1 value
    struct TypedValue x1Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (x1Value.type == ValueTypeError) return x1Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y1 value
    struct TypedValue y1Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (y1Value.type == ValueTypeError) return y1Value.v.errorCode;
    
    // TO
    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
    ++interpreter->pc;
    
    // x2 value
    struct TypedValue x2Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (x2Value.type == ValueTypeError) return x2Value.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y2 value
    struct TypedValue y2Value = itp_evaluateExpression(core, TypeClassNumeric);
    if (y2Value.type == ValueTypeError) return y2Value.v.errorCode;
    
    struct SimpleAttributes attrs;
    enum ErrorCode attrsError = itp_evaluateSimpleAttributes(core, &attrs);
    if (attrsError != ErrorNone) return attrsError;
    
    if (interpreter->pass == PassRun)
    {
        txtlib_setCellsAttr(&interpreter->textLib, floorf(x1Value.v.floatValue), floorf(y1Value.v.floatValue), floorf(x2Value.v.floatValue), floorf(y2Value.v.floatValue), attrs.pal, attrs.flipX, attrs.flipY, attrs.prio);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_CELL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // CELL
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
    
    // character value
    struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
    if (cValue.type == ValueTypeError) return cValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int c = (cValue.type == ValueTypeFloat) ? cValue.v.floatValue : -1;
        txtlib_setCell(&interpreter->textLib, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue), c);
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
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        struct Cell *cell = txtlib_getCell(&interpreter->textLib, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
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

enum ErrorCode cmd_MCELL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // MCELL
    ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateNumericExpression(core, 0, interpreter->textLib.sourceWidth - 1);
    if (xValue.type == ValueTypeError) return xValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateNumericExpression(core, 0, interpreter->textLib.sourceHeight - 1);
    if (yValue.type == ValueTypeError) return yValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // character value
    struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
    if (cValue.type == ValueTypeError) return cValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int c = (cValue.type == ValueTypeFloat) ? cValue.v.floatValue : -1;
        bool success = txtlib_setSourceCell(&interpreter->textLib, xValue.v.floatValue, yValue.v.floatValue, c);
        if (!success) return ErrorIllegalMemoryAccess;
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_MCELL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // MCELL.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;
    
    // x value
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
        ++interpreter->pc;
    
    // y value
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int x = floorf(xValue.v.floatValue);
        int y = floorf(yValue.v.floatValue);
        value.v.floatValue = txtlib_getSourceCell(&interpreter->textLib, x, y, (type == TokenMCELLA));
    }
    return value;
}

enum ErrorCode cmd_TINT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // TINT
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
    
    struct SimpleAttributes attrs;
    enum ErrorCode attrsError = itp_evaluateSimpleAttributes(core, &attrs);
    if (attrsError != ErrorNone) return attrsError;
    
    if (interpreter->pass == PassRun)
    {
        struct Cell *cell = txtlib_getCell(&interpreter->textLib, floorf(xValue.v.floatValue), floorf(yValue.v.floatValue));
        if (attrs.pal >= 0) cell->attr.palette = attrs.pal;
        if (attrs.flipX >= 0) cell->attr.flipX = attrs.flipX;
        if (attrs.flipY >= 0) cell->attr.flipY = attrs.flipY;
        if (attrs.prio >= 0) cell->attr.priority = attrs.prio;
    }
    
    return itp_endOfCommand(interpreter);
}
