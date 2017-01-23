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
#include "system_ram.h"

int tick = 0;

void LRC_init(struct LowResCore *core)
{
    LRC_initMachine(&core->machine);
    
    core->machine.videoRegisters.attr_romCells = 1;
    
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
    
    struct SystemRam *systemRam = (struct SystemRam *)core->machine.workingRam;
    struct TextLib *textLib = &systemRam->textLib;
    textLib->attr_bank = 1;
    textLib->attr_priority = 1;
    textLib->attr_palette = 7;
    textLib->characterOffset = 128;
    textLib->areaX = 0;
    textLib->areaY = 12;
    textLib->areaWidth = 16;
    textLib->areaHeight = 4;
    LRC_writeText(&core->machine, "SCORE", 0, 0);
    
    memcpy(&core->machine.videoRam.characterBanks[0], DemoCharacters, sizeof(DemoCharacters));
    memcpy(core->machine.videoRam.planeB.cells, DemoBackground, sizeof(DemoBackground));
    memcpy(core->machine.videoRam.planeA.cells, DemoMap, sizeof(DemoMap));
    
    struct Sprite *sprite = &core->machine.videoRam.sprites[0];
    sprite->character = 128;
    sprite->x = 64;
    sprite->y = 96;
    sprite->attr_palette = 6;
    sprite->attr_priority = 0;
    sprite->attr_width = 1;
    sprite->attr_height = 1;
}

void LRC_update(struct LowResCore *core)
{
    struct SystemRam *systemRam = (struct SystemRam *)core->machine.workingRam;
    struct TextLib *textLib = &systemRam->textLib;
    
    core->machine.videoRegisters.scrollBX = tick;
    core->machine.videoRegisters.scrollAX = tick * 2;
    
    struct Sprite *sprite = &core->machine.videoRam.sprites[0];
    sprite->character = 130 + ((tick/4)%3) * 2;
    
    if (tick%15 == 0)
    {
        textLib->attr_palette = rand() % 8;
        textLib->attr_flipX = rand()%2;
        textLib->attr_flipY = rand()%2;
        LRC_printText(&core->machine, "PRINT TEXT! ");
    }
    textLib->attr_palette = 7;
    textLib->attr_flipX = 0;
    textLib->attr_flipY = 0;
    LRC_writeNumber(&core->machine, tick/10, 5, 11, 0);

    tick++;
}
