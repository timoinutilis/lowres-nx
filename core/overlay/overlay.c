//
// Copyright 2017-2018 Timo Kloss
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
    lib->fontCharOffset = 0;
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

void overlay_message(struct Core *core, const char *message)
{
    struct TextLib *lib = &core->overlay->textLib;
    txtlib_setCells(lib, 0, 15, 19, 15, 0);
    txtlib_writeText(lib, message, 0, 15);
    core->overlay->messageTimer = 120;
    machine_suspendEnergySaving(core, 120);
}

void overlay_draw(struct Core *core, bool ingame)
{
    struct TextLib *lib = &core->overlay->textLib;
    
    if (core->overlay->messageTimer > 0)
    {
        core->overlay->messageTimer--;
        if (core->overlay->messageTimer < 20)
        {
            txtlib_scrollBackground(lib, 0, 15, 19, 15, -1, 0);
            txtlib_setCell(lib, 19, 15, 0);
        }
    }
    
    if (ingame)
    {
        if (core->interpreter->state == StatePaused)
        {
            if (core->overlay->timer % 60 < 40)
            {
                txtlib_writeText(lib, "PAUSED", 7, 7);
            }
            else
            {
                txtlib_setCells(lib, 7, 7, 12, 7, 0);
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
    core->overlay->messageTimer = 0;
}

