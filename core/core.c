//
// Copyright 2016 Timo Kloss
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

#include "core.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>

void core_init(struct Core *core)
{
    memset(core, 0, sizeof(struct Core));
    
    machine_init(core);
    overlay_init(core);
    disk_init(core);
}

void core_deinit(struct Core *core)
{
    disk_deinit(core);
}

void core_setDelegate(struct Core *core, struct CoreDelegate *delegate)
{
    core->delegate = delegate;
}

void core_willRunProgram(struct Core *core, long secondsSincePowerOn)
{
    core->interpreter.timer = (float)(secondsSincePowerOn * 30 % TIMER_WRAP_VALUE);
}

void core_update(struct Core *core)
{
    itp_runProgram(core);
    itp_runInterrupt(core, InterruptTypeVBL);
    itp_didFinishVBL(core);
    overlay_drawButtons(core);
}

void core_keyPressed(struct Core *core, char key)
{
    if (core->machine.ioRegisters.attr.keyboardEnabled)
    {
        if (key >= 32 && key < 127)
        {
            core->machine.ioRegisters.key = key;
        }
    }
    else
    {
        if (key == 'p' || key == 'P')
        {
            core->machine.ioRegisters.status.pause = 1;
        }
    }
}

void core_backspacePressed(struct Core *core)
{
    if (core->machine.ioRegisters.attr.keyboardEnabled)
    {
        core->machine.ioRegisters.key = '\b';
    }
}

void core_returnPressed(struct Core *core)
{
    if (core->machine.ioRegisters.attr.keyboardEnabled)
    {
        core->machine.ioRegisters.key = '\n';
    }
}

void core_touchPressed(struct Core *core, int x, int y, const void *touchReference)
{
    if (core->machine.ioRegisters.attr.gamepadsEnabled)
    {
        overlay_touchPressed(core, x, y, touchReference);
    }
    else
    {
        core->machine.ioRegisters.status.touch = 1;
        core_touchDragged(core, x, y, touchReference);
    }
}

void core_touchDragged(struct Core *core, int x, int y, const void *touchReference)
{
    if (core->machine.ioRegisters.attr.gamepadsEnabled)
    {
        overlay_touchDragged(core, x, y, touchReference);
    }
    else if (core->machine.ioRegisters.status.touch)
    {
        if (x < 0) x = 0; else if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
        if (y < 0) y = 0; else if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;
        core->machine.ioRegisters.touchX = x;
        core->machine.ioRegisters.touchY = y;
    }
}

void core_touchReleased(struct Core *core, const void *touchReference)
{
    if (core->machine.ioRegisters.attr.gamepadsEnabled)
    {
        overlay_touchReleased(core, touchReference);
    }
    else
    {
        core->machine.ioRegisters.status.touch = 0;
    }
}

void core_gamepadPressed(struct Core *core, int player, enum GamepadButton button)
{
    if (core->machine.ioRegisters.attr.gamepadsEnabled)
    {
        switch (button)
        {
            case GamepadButtonUp:
                core->machine.ioRegisters.gamepads[player].up = 1;
                break;
            case GamepadButtonDown:
                core->machine.ioRegisters.gamepads[player].down = 1;
                break;
            case GamepadButtonLeft:
                core->machine.ioRegisters.gamepads[player].left = 1;
                break;
            case GamepadButtonRight:
                core->machine.ioRegisters.gamepads[player].right = 1;
                break;
            case GamepadButtonA:
                core->machine.ioRegisters.gamepads[player].buttonA = 1;
                break;
            case GamepadButtonB:
                core->machine.ioRegisters.gamepads[player].buttonB = 1;
                break;
        }
    }
}

void core_gamepadReleased(struct Core *core, int player, enum GamepadButton button)
{
    if (core->machine.ioRegisters.attr.gamepadsEnabled)
    {
        switch (button)
        {
            case GamepadButtonUp:
                core->machine.ioRegisters.gamepads[player].up = 0;
                break;
            case GamepadButtonDown:
                core->machine.ioRegisters.gamepads[player].down = 0;
                break;
            case GamepadButtonLeft:
                core->machine.ioRegisters.gamepads[player].left = 0;
                break;
            case GamepadButtonRight:
                core->machine.ioRegisters.gamepads[player].right = 0;
                break;
            case GamepadButtonA:
                core->machine.ioRegisters.gamepads[player].buttonA = 0;
                break;
            case GamepadButtonB:
                core->machine.ioRegisters.gamepads[player].buttonB = 0;
                break;
        }
    }
}

void core_setGamepad(struct Core *core, int player, bool up, bool down, bool left, bool right, bool buttonA, bool buttonB)
{
    if (core->machine.ioRegisters.attr.gamepadsEnabled)
    {
        union Gamepad *gamepad = &core->machine.ioRegisters.gamepads[player];
        gamepad->up = up;
        gamepad->down = down;
        gamepad->left = left;
        gamepad->right = right;
        gamepad->buttonA = buttonA;
        gamepad->buttonB = buttonB;
    }
}

bool core_getKeyboardEnabled(struct Core *core)
{
    return core->machine.ioRegisters.attr.keyboardEnabled;
}
