//
// Copyright 2017 Timo Kloss
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

#include "cmd_io.h"
#include "core.h"
#include <assert.h>

enum ErrorCode cmd_KEYBOARD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // KEYBOARD
    ++interpreter->pc;
    
    // ON/OFF/OPTIONAL
    enum TokenType type = interpreter->pc->type;
    if (type != TokenON && type != TokenOFF && type != TokenOPTIONAL) return ErrorSyntax;
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        core->machine->ioRegisters.attr.keyboardEnabled = (type == TokenON || type == TokenOPTIONAL);
        interpreter->isKeyboardOptional = (type == TokenOPTIONAL);
        delegate_controlsDidChange(core);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_TOUCHSCREEN(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // TOUCHSCREEN
    ++interpreter->pc;
        
    if (interpreter->pass == PassRun)
    {
        if (core->machine->ioRegisters.attr.gamepadsEnabled > 0) return ErrorInputChangeNotAllowed;
        core->machine->ioRegisters.attr.touchEnabled = 1;
        delegate_controlsDidChange(core);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_GAMEPAD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // GAMEPAD
    ++interpreter->pc;
    
    // number
    struct TypedValue value = itp_evaluateNumericExpression(core, 1, 2);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        if (core->machine->ioRegisters.attr.touchEnabled) return ErrorInputChangeNotAllowed;
        
        core->machine->ioRegisters.attr.gamepadsEnabled = value.v.floatValue;
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
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // p expression
    struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 1);
    if (pValue.type == ValueTypeError) return pValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        if (core->machine->ioRegisters.attr.gamepadsEnabled == 0) return val_makeError(ErrorGamepadNotEnabled);
        
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
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // p expression
    struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 1);
    if (pValue.type == ValueTypeError) return pValue;
    
    int n = -1;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
    
        // n expression
        struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, 1);
        if (nValue.type == ValueTypeError) return nValue;
        
        n = nValue.v.floatValue;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        if (core->machine->ioRegisters.attr.gamepadsEnabled == 0) return val_makeError(ErrorGamepadNotEnabled);
        
        int p = pValue.v.floatValue;
        union Gamepad *gamepad = &core->machine->ioRegisters.gamepads[p];

        int active = (n == -1) ? (gamepad->buttonA || gamepad->buttonB) : (n == 0) ? gamepad->buttonA : gamepad->buttonB;
        
        if (active && tap)
        {
            // invalidate button if it was already pressed last frame
            union Gamepad *lastFrameGamepad = &core->interpreter->lastFrameGamepads[p];
            if ((n == -1) ? (lastFrameGamepad->buttonA || lastFrameGamepad->buttonB) : (n == 0) ? lastFrameGamepad->buttonA : lastFrameGamepad->buttonB)
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
        if (core->machine->ioRegisters.attr.touchEnabled == 0) return val_makeError(ErrorTouchNotEnabled);
        
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
        if (core->machine->ioRegisters.attr.touchEnabled == 0) return val_makeError(ErrorTouchNotEnabled);
        
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
        if (core->machine->ioRegisters.attr.touchEnabled == 0) return val_makeError(ErrorTouchNotEnabled);
        
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
