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

#include "cmd_io.h"
#include "core.h"
#include <assert.h>

struct TypedValue fnc_UP_DOWN_LEFT_RIGHT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // UP/DOWN/LEFT/RIGHT
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // p expression
    struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 1);
    if (pValue.type == ValueTypeError) return pValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int p = pValue.v.floatValue;
        union Gamepad *gamepad = &core->machine.ioRegisters.gamepads[p];
        switch (type)
        {
            case TokenUP:
                value.v.floatValue = gamepad->status_up ? BAS_TRUE : BAS_FALSE;
                break;
                
            case TokenDOWN:
                value.v.floatValue = gamepad->status_down ? BAS_TRUE : BAS_FALSE;
                break;

            case TokenLEFT:
                value.v.floatValue = gamepad->status_left ? BAS_TRUE : BAS_FALSE;
                break;

            case TokenRIGHT:
                value.v.floatValue = gamepad->status_right ? BAS_TRUE : BAS_FALSE;
                break;
                
            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_BUTTON(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // BUTTON
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // p expression
    struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 1);
    if (pValue.type == ValueTypeError) return pValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
    ++interpreter->pc;
    
    // n expression
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, 1);
    if (nValue.type == ValueTypeError) return nValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int p = pValue.v.floatValue;
        int n = nValue.v.floatValue;
        union Gamepad *gamepad = &core->machine.ioRegisters.gamepads[p];
        if (n == 0)
        {
            value.v.floatValue = gamepad->status_buttonA ? BAS_TRUE : BAS_FALSE;
        }
        else
        {
            value.v.floatValue = gamepad->status_buttonB ? BAS_TRUE : BAS_FALSE;
        }
    }
    return value;
}

struct TypedValue fnc_TOUCH(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // TOUCH.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        if (type == TokenTOUCH)
        {
            value.v.floatValue = core->machine.ioRegisters.status_touch ? BAS_TRUE : BAS_FALSE;
        }
        else if (type == TokenTOUCHX)
        {
            value.v.floatValue = core->machine.ioRegisters.touchX;
        }
        else if (type == TokenTOUCHY)
        {
            value.v.floatValue = core->machine.ioRegisters.touchY;
        }
        else
        {
            assert(0);
        }
    }
    return value;
}
