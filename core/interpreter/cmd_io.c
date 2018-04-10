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

enum ErrorCode cmd_KEYBOARD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // KEYBOARD
    ++interpreter->pc;
    
    // ON/OFF
    enum TokenType type = interpreter->pc->type;
    if (type != TokenON && type != TokenOFF) return ErrorUnexpectedToken;
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        core->machine->ioRegisters.attr.keyboardEnabled = (type == TokenON);
        delegate_controlsDidChange(core);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_GAMEPAD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // GAMEPAD
    ++interpreter->pc;
    
    int num = 0;
    if (interpreter->pc->type == TokenOFF)
    {
        // OFF
        ++interpreter->pc;
    }
    else
    {
        // number
        struct TypedValue value = itp_evaluateNumericExpression(core, 0, 2);
        if (value.type == ValueTypeError) return value.v.errorCode;
        num = value.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        core->machine->ioRegisters.attr.gamepadsEnabled = num;
        core->machine->ioRegisters.status.touch = 0;
        for (int i = 0; i < NUM_GAMEPADS; i++)
        {
            core->machine->ioRegisters.gamepads[i].value = 0;
        }
        delegate_controlsDidChange(core);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_PAUSE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // PAUSE
    ++interpreter->pc;
    
    // ON/OFF?
    enum TokenType type = interpreter->pc->type;
    if (type == TokenON || type == TokenOFF)
    {
        ++interpreter->pc;
    }
    
    if (interpreter->pass == PassRun)
    {
        if (type == TokenON)
        {
            core->machine->ioRegisters.status.pause = 0;
            interpreter->handlesPause = true;
        }
        else if (type == TokenOFF)
        {
            interpreter->handlesPause = false;
        }
        else
        {
            interpreter->state = StatePaused;
            overlay_updateState(core);
        }
    }
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_UP_DOWN_LEFT_RIGHT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // UP/DOWN/LEFT/RIGHT
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // TAP
    bool tap = false;
    if (interpreter->pc->type == TokenTAP)
    {
        ++interpreter->pc;
        tap = true;
    }
    
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
        int active = 0;
        int lastFrameActive = 0;
        union Gamepad *gamepad = &core->machine->ioRegisters.gamepads[p];
        union Gamepad *lastFrameGamepad = &core->interpreter->lastFrameGamepads[p];
        switch (type)
        {
            case TokenUP:
                active = gamepad->up;
                lastFrameActive = lastFrameGamepad->up;
                break;
                
            case TokenDOWN:
                active = gamepad->down;
                lastFrameActive = lastFrameGamepad->down;
                break;

            case TokenLEFT:
                active = gamepad->left;
                lastFrameActive = lastFrameGamepad->left;
                break;

            case TokenRIGHT:
                active = gamepad->right;
                lastFrameActive = lastFrameGamepad->right;
                break;
                
            default:
                assert(0);
                break;
        }
        value.v.floatValue = active && !(tap && lastFrameActive) ? BAS_TRUE : BAS_FALSE;
    }
    return value;
}

struct TypedValue fnc_BUTTON(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BUTTON
    ++interpreter->pc;
    
    // TAP
    bool tap = false;
    if (interpreter->pc->type == TokenTAP)
    {
        ++interpreter->pc;
        tap = true;
    }
    
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
        union Gamepad *gamepad = &core->machine->ioRegisters.gamepads[p];

        int active = (n == 0) ? gamepad->buttonA : gamepad->buttonB;
        
        if (active && tap)
        {
            // invalidate button if it was already pressed last frame
            union Gamepad *lastFrameGamepad = &core->interpreter->lastFrameGamepads[p];
            if ((n == 0) ? lastFrameGamepad->buttonA : lastFrameGamepad->buttonB)
            {
                active = 0;
            }
        }
        
        value.v.floatValue = active ? BAS_TRUE : BAS_FALSE;
    }
    return value;
}

struct TypedValue fnc_TOUCH(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // TOUCH
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = core->machine->ioRegisters.status.touch ? BAS_TRUE : BAS_FALSE;
    }
    return value;
}

struct TypedValue fnc_TAP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // TAP
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = (core->machine->ioRegisters.status.touch && !core->interpreter->lastFrameIOStatus.touch) ? BAS_TRUE : BAS_FALSE;
    }
    return value;
}

struct TypedValue fnc_TOUCH_X_Y(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // TOUCH.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        if (type == TokenTOUCHX)
        {
            value.v.floatValue = core->machine->ioRegisters.touchX;
        }
        else if (type == TokenTOUCHY)
        {
            value.v.floatValue = core->machine->ioRegisters.touchY;
        }
        else
        {
            assert(0);
        }
    }
    return value;
}

struct TypedValue fnc_PAUSE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // PAUSE
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        if (interpreter->handlesPause) return val_makeError(ErrorAutomaticPauseNotDisabled);
        
        value.v.floatValue = core->machine->ioRegisters.status.pause ? BAS_TRUE : BAS_FALSE;
        core->machine->ioRegisters.status.pause = 0;
    }
    return value;
}
