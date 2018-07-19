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
#include <math.h>

void overlay_clear(struct Core *core);


void overlay_init(struct Core *core)
{
    struct TextLib *lib = &core->overlay->textLib;
    lib->core = core;
    lib->bg = OVERLAY_BG;
    lib->windowBg = OVERLAY_BG;
    lib->charAttr.priority = 1;
    lib->charAttr.palette = 1;
    lib->charAttrFilter = 0xFF;
    lib->fontCharOffset = 64;
    lib->windowX = 0;
    lib->windowY = 0;
    lib->windowWidth = 20;
    lib->windowHeight = 16;
    lib->cursorX = 0;
    lib->cursorY = 0;
}

void overlay_reset(struct Core *core)
{
    overlay_clear(core);
    core->overlay->textLib.cursorX = 0;
    core->overlay->textLib.cursorY = 0;
}

void overlay_updateState(struct Core *core)
{
    overlay_clear(core);
    
    if (core->interpreter->state == StatePaused)
    {
        core->overlay->timer = 0;
    }
    
    if (!core->interpreter->debug)
    {
        core->overlay->textLib.cursorX = 0;
        core->overlay->textLib.cursorY = 0;
    }
}

void overlay_draw(struct Core *core)
{
    struct TextLib *lib = &core->overlay->textLib;
    if (core->interpreter->state == StatePaused)
    {
        if (core->overlay->timer % 60 < 40)
        {
            txtlib_writeText(lib, "PAUSED", 7, 7);
        }
        else
        {
            txtlib_writeText(lib, "      ", 7, 7);
        }
    }
    
    if (core->interpreter->debug)
    {
        txtlib_writeText(lib, "CPU", 17, 0);
        int cpuLoad = core->interpreter->cpuLoadDisplay;
        if (cpuLoad < 100)
        {
            txtlib_writeNumber(lib, cpuLoad, 2, 17, 1);
            txtlib_writeText(lib, "%", 19, 1);
        }
        else
        {
            txtlib_writeText(lib, "MAX", 17, 1);
        }
    }
    
    core->overlay->timer++;
}

void overlay_clear(struct Core *core)
{
    struct Plane *plane = &core->overlay->plane;
    for (int y = 0; y < PLANE_ROWS; y++)
    {
        for (int x = 0; x < PLANE_COLUMNS; x++)
        {
            struct Cell *cell = &plane->cells[y][x];
            cell->character = 0;
            cell->attr.palette = 0;
            cell->attr.priority = 1;
        }
    }
}

