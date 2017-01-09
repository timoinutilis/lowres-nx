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

int tick = 0;

void setChar(Window *window, int x, int y, char character)
{
    Cell *cell = &window->cells[y][x];
    cell->character = character + 32 + 64;
    cell->attr_bank = 1;
    cell->attr_priority = 1;
    cell->attr_palette = 7;
}

void LRC_init(LRCore *core)
{
    uint8_t *colors = core->videoInterface.colors;
    colors[0] = (0<<4) | (1<<2) | 3;
    
    colors[1] = (1<<4) | (0<<2) | 0;
    colors[2] = (2<<4) | (1<<2) | 0;
    colors[3] = (3<<4) | (2<<2) | 0;
    
    colors[5] = (0<<4) | (1<<2) | 0;
    colors[6] = (0<<4) | (2<<2) | 0;
    colors[7] = (0<<4) | (3<<2) | 0;

    colors[9]  = (0<<4) | (1<<2) | 3;
    colors[10] = (0<<4) | (2<<2) | 3;
    colors[11] = (0<<4) | (3<<2) | 3;

    colors[13] = (1<<4) | (2<<2) | 3;
    colors[14] = (2<<4) | (3<<2) | 3;
    colors[15] = (3<<4) | (3<<2) | 3;

    colors[25] = (1<<4) | (0<<2) | 0;
    colors[26] = (3<<4) | (0<<2) | 0;
    colors[27] = (3<<4) | (2<<2) | 1;

    colors[29] = 0;
    colors[30] = (1<<4) | (1<<2) | 1;
    colors[31] = (3<<4) | (3<<2) | 3;
    
    Window *window = &core->videoInterface.window;
    setChar(window, 0, 0, 'S');
    setChar(window, 1, 0, 'C');
    setChar(window, 2, 0, 'O');
    setChar(window, 3, 0, 'R');
    setChar(window, 4, 0, 'E');
    setChar(window, 12, 0, '4');
    setChar(window, 13, 0, '2');
    setChar(window, 14, 0, '0');
    setChar(window, 15, 0, '0');
    
    memcpy(core->videoInterface.characters, DemoCharacters, sizeof(DemoCharacters));
    memcpy(core->videoInterface.planes[0].cells, DemoBackground, sizeof(DemoBackground));
    memcpy(core->videoInterface.planes[1].cells, DemoMap, sizeof(DemoMap));
    
    Sprite *sprite = &core->videoInterface.sprites[0];
    sprite->character = 128;
    sprite->x = 96;
    sprite->y = 96;
    sprite->attr_palette = 6;
    sprite->attr_priority = 0;
    sprite->attr_width = 1;
    sprite->attr_height = 1;
}

void LRC_update(LRCore *core)
{
    core->videoInterface.planes[0].scrollX = tick;
    core->videoInterface.planes[1].scrollX = tick * 2;
    
    Sprite *sprite = &core->videoInterface.sprites[0];
    sprite->character = 130 + ((tick/4)%3) * 2;
    tick++;

}
