//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#include "lowres_core.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "demo_data.h"
#include "text_lib.h"

int tick = 0;

void normalPalette(struct LowResCore *core)
{
    uint8_t *colors = core->machine.videoRegisters.colors;
    colors[0] = (0<<4) | (1<<2) | 3;
    
    // 0 brown
    colors[1] = (1<<4) | (0<<2) | 0;
    colors[2] = (2<<4) | (1<<2) | 0;
    colors[3] = (3<<4) | (2<<2) | 0;
    
    // 1 green
    colors[5] = (0<<4) | (1<<2) | 0;
    colors[6] = (0<<4) | (2<<2) | 0;
    colors[7] = (0<<4) | (3<<2) | 0;
    
    // 2 sea blue
    colors[9]  = (0<<4) | (1<<2) | 3;
    colors[10] = (0<<4) | (2<<2) | 3;
    colors[11] = (0<<4) | (3<<2) | 3;
    
    // 3 ?
    colors[13] = (1<<4) | (2<<2) | 3;
    colors[14] = (2<<4) | (3<<2) | 3;
    colors[15] = (3<<4) | (3<<2) | 3;
    
    // 4 orange
    colors[17] = (3<<4) | (1<<2) | 0;
    colors[18] = (3<<4) | (2<<2) | 0;
    colors[19] = (3<<4) | (3<<2) | 0;
    
    // 5 red
    colors[21] = (1<<4) | (0<<2) | 0;
    colors[22] = (2<<4) | (0<<2) | 0;
    colors[23] = (3<<4) | (0<<2) | 0;
    
    // 6 mario sprite
    colors[25] = (1<<4) | (0<<2) | 0;
    colors[26] = (3<<4) | (0<<2) | 0;
    colors[27] = (3<<4) | (2<<2) | 1;
    
    // 7 white text
    colors[29] = 0;
    colors[30] = (1<<4) | (1<<2) | 1;
    colors[31] = (3<<4) | (3<<2) | 3;
}

void reflectionPalette(struct LowResCore *core)
{
    uint8_t *colors = core->machine.videoRegisters.colors;
    colors[0] = (1<<2) | 3;
    
    // 0 brown
    colors[1] = (1<<2) | 3;
    colors[2] = (2<<2) | 3;
    colors[3] = (3<<2) | 3;
    
    // 1 green
    colors[5] = (2<<2) | 3;
    colors[6] = (3<<2) | 3;
    colors[7] = (3<<2) | 3;
    
    // 2 sea blue
    colors[9]  = (2<<2) | 3;
    colors[10] = (3<<2) | 3;
    colors[11] = (3<<2) | 3;
    
    // 3 ?
    colors[13] = (3<<2) | 3;
    colors[14] = (3<<2) | 3;
    colors[15] = (3<<2) | 3;
    
    // 4 orange
    colors[17] = (2<<2) | 3;
    colors[18] = (3<<2) | 3;
    colors[19] = (3<<2) | 3;
    
    // 5 red
    colors[21] = (1<<2) | 3;
    colors[22] = (1<<2) | 3;
    colors[23] = (1<<2) | 3;
}

void LRC_init(struct LowResCore *core)
{
    LRC_initMachine(&core->machine);
    
    struct TextLib *textLib = &core->interpreter.textLib;
    textLib->charAttr.bank = 1;
    textLib->charAttr.priority = 1;
    textLib->charAttr.palette = 7;
    textLib->characterOffset = 128;
    
    memcpy(&core->machine.videoRam.characterBank, DemoCharacters, sizeof(DemoCharacters));
    memcpy(core->machine.videoRam.planeB.cells, DemoBackground, sizeof(DemoBackground));
    memcpy(core->machine.videoRam.planeA.cells, DemoMap, sizeof(DemoMap));
    
    LRC_writeText(core, "SCORE", 0, 0);
    
    struct Sprite *sprite = &core->machine.videoRegisters.sprites[0];
    sprite->character = 128;
    sprite->x = 64;
    sprite->y = 96;
    sprite->attr1.palette = 6;
    sprite->attr1.priority = 0;
    sprite->attr2.width = 1;
    sprite->attr2.height = 1;
}

void LRC_update(struct LowResCore *core)
{
    core->machine.videoRegisters.scrollAX = tick * 2;
    
    struct Sprite *sprite = &core->machine.videoRegisters.sprites[0];
    sprite->character = 130 + ((tick/4)%3) * 2;
    
    LRC_writeNumber(core, tick/10, 5, 11, 0);

    tick++;
}

void LRC_rasterUpdate(struct LowResCore *core)
{
    int y = core->machine.videoRegisters.rasterLine;
    int wy = 112+sin(tick*0.02)*16;
    if (y == 0)
    {
        normalPalette(core);
    }
    else if (y == wy)
    {
        reflectionPalette(core);
    }

    uint8_t *colors = core->machine.videoRegisters.colors;
    int cy = y + (y%2 * 4);
    if (cy < 12)
    {
        colors[0] = (3<<4) | (1<<2) | 3;
    }
    else if (cy < 24)
    {
        colors[0] = (2<<4) | (1<<2) | 3;
    }
    else if (cy < 36)
    {
        colors[0] = (1<<4) | (1<<2) | 3;
    }
    else
    {
        colors[0] = (0<<4) | (1<<2) | 3;
    }

    if (y < 8)
    {
        core->machine.videoRegisters.scrollBX = 0;
    }
    else if (y < 32)
    {
        core->machine.videoRegisters.scrollBX = tick * 3 / 4;
    }
    else if (y < 56)
    {
        core->machine.videoRegisters.scrollBX = tick / 2;
    }
    else if (y > 56 + 3*8)
    {
        core->machine.videoRegisters.scrollBX = tick * 3;
    }
    else
    {
        core->machine.videoRegisters.scrollBX = tick * (0.5 + (y - 55.0) / 32.0);
    }
    if (y > wy)
    {
        core->machine.videoRegisters.scrollAX += (tick/8 + y)%3;
        core->machine.videoRegisters.scrollBX += (tick/8 + y)%3;
    }
}
