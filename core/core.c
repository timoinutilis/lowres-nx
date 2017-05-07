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
#include "text_lib.h"

void core_init(struct Core *core)
{
    memset(core, 0, sizeof(struct Core));
    
    machine_init(&core->machine);
    txtlib_init(core);
}

void core_update(struct Core *core)
{
    itp_runProgram(core);
    itp_runInterrupt(core, InterruptTypeVBL);
}

void core_rasterUpdate(struct Core *core)
{
    itp_runInterrupt(core, InterruptTypeRaster);
}

void core_keyPressed(struct Core *core, char key)
{
    if (key >= 32 && key < 127)
    {
        core->machine.ioRegisters.key = key;
    }
}

void core_backspacePressed(struct Core *core)
{
    core->machine.ioRegisters.key = '\b';
}

void core_returnPressed(struct Core *core)
{
    core->machine.ioRegisters.key = '\n';
}

void core_mouseMoved(struct Core *core, int x, int y)
{
    if (x < 0) x = 0; else if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
    if (y < 0) y = 0; else if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;
    core->machine.ioRegisters.mouseX = x;
    core->machine.ioRegisters.mouseY = y;
}

void core_mousePressed(struct Core *core)
{
    core->machine.ioRegisters.status_mouseButton = 1;
}

void core_mouseReleased(struct Core *core)
{
    core->machine.ioRegisters.status_mouseButton = 0;
}

void core_gamepadPressed(struct Core *core, int player, enum GamepadButton button)
{
    switch (button)
    {
        case GamepadButtonUp:
            core->machine.ioRegisters.gamepads[player].status_up = 1;
            break;
        case GamepadButtonDown:
            core->machine.ioRegisters.gamepads[player].status_down = 1;
            break;
        case GamepadButtonLeft:
            core->machine.ioRegisters.gamepads[player].status_left = 1;
            break;
        case GamepadButtonRight:
            core->machine.ioRegisters.gamepads[player].status_right = 1;
            break;
        case GamepadButtonA:
            core->machine.ioRegisters.gamepads[player].status_buttonA = 1;
            break;
        case GamepadButtonB:
            core->machine.ioRegisters.gamepads[player].status_buttonB = 1;
            break;
    }
}

void core_gamepadReleased(struct Core *core, int player, enum GamepadButton button)
{
    switch (button)
    {
        case GamepadButtonUp:
            core->machine.ioRegisters.gamepads[player].status_up = 0;
            break;
        case GamepadButtonDown:
            core->machine.ioRegisters.gamepads[player].status_down = 0;
            break;
        case GamepadButtonLeft:
            core->machine.ioRegisters.gamepads[player].status_left = 0;
            break;
        case GamepadButtonRight:
            core->machine.ioRegisters.gamepads[player].status_right = 0;
            break;
        case GamepadButtonA:
            core->machine.ioRegisters.gamepads[player].status_buttonA = 0;
            break;
        case GamepadButtonB:
            core->machine.ioRegisters.gamepads[player].status_buttonB = 0;
            break;
    }
}
