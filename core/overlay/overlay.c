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

#include "overlay.h"
#include "core.h"
#include "io_chip.h"

void overlay_init(struct Core *core)
{
    struct Plane *plane = &core->overlay.plane;
    for (int y = 0; y < PLANE_ROWS; y++)
    {
        for (int x = 0; x < PLANE_COLUMNS; x++)
        {
            plane->cells[y][x].attr.palette = 0;
            plane->cells[y][x].attr.priority = 1;
        }
    }
}

void overlay_drawDPad(struct Plane *plane, int x, int y, union Gamepad gamepad)
{
    plane->cells[y][x].character = 16;
    plane->cells[y][x+1].character = gamepad.up ? 20 : 17;
    plane->cells[y][x+2].character = 18;
    plane->cells[y+1][x].character = gamepad.left ? 35 : 32;
    plane->cells[y+1][x+1].character = 33;
    plane->cells[y+1][x+2].character = gamepad.right ? 37 : 34;
    plane->cells[y+2][x].character = 48;
    plane->cells[y+2][x+1].character = gamepad.down ? 52 : 49;
    plane->cells[y+2][x+2].character = 50;
}

void overlay_drawButton(struct Plane *plane, int x, int y, int character)
{
    plane->cells[y][x].character = character;
    plane->cells[y][x+1].character = character + 1;
    plane->cells[y+1][x].character = character + 16;
    plane->cells[y+1][x+1].character = character + 17;
}

void overlay_drawGamepads(struct Core *core)
{
    struct Plane *plane = &core->overlay.plane;
    union Gamepad gamepad0 = core->machine.ioRegisters.gamepads[0];
    overlay_drawDPad(plane, 1, 12, gamepad0);
    overlay_drawButton(plane, 15, 13, gamepad0.buttonA ? 40 : 38);
    overlay_drawButton(plane, 17, 12, gamepad0.buttonB ? 44 : 42);
    overlay_drawButton(plane, 18, 14, core->machine.ioRegisters.status.pause ? 8 : 6);
}
