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

float rotation = 0;

void setChar(Window *window, int x, int y, char character)
{
    Cell *cell = &window->cells[y][x];
    cell->character = character + 32 + 64;
    cell->attr_bank = 2;
    cell->attr_priority = 1;
    cell->attr_palette = 4;
}

void LRC_init(LRCore *core)
{
    core->videoInterface.colors[0] = 3;
    core->videoInterface.colors[1] = 12;
    core->videoInterface.colors[2] = 48;
    core->videoInterface.colors[3] = 63;
    core->videoInterface.colors[5] = 20;
    core->videoInterface.colors[6] = 25;
    core->videoInterface.colors[7] = 12;
    core->videoInterface.colors[11] = 48;
    core->videoInterface.colors[15] = 60;
    core->videoInterface.colors[17] = 0;
    core->videoInterface.colors[19] = 63;
    
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
    
    Sprite *sprite = &core->videoInterface.sprites[0];
    sprite->character = 128+32;
    sprite->attr_palette = 4;
    sprite->attr_priority = 1;
    sprite->attr_bank = 2;

    sprite = &core->videoInterface.sprites[1];
    sprite->character = 129;
    sprite->attr_palette = 4;
    sprite->attr_priority = 0;
    sprite->attr_bank = 2;
    sprite->attr_width = 3;
    sprite->attr_height = 3;

    for (int i = 0; i < 640; i++)
    {
        int pli = i%2;
        Cell *cell = &core->videoInterface.planes[pli].cells[rand()%32][rand()%32];
        cell->character = 64+(rand()%48);
        cell->attr_palette = rand()%4;
        cell->attr_priority = rand()%2;
        cell->attr_bank = 2;
    }
}

void LRC_update(LRCore *core)
{
    core->videoInterface.planes[0].scrollX--;
    core->videoInterface.planes[1].scrollY--;
    
    Sprite *sprite = &core->videoInterface.sprites[0];
    sprite->x += 2;
    sprite->y += 2;
    
    rotation += 0.1;
    
    sprite = &core->videoInterface.sprites[1];
    sprite->x = 64 + SPRITE_OFFSET_X - 16 + sinf(rotation) * 32;
    sprite->y = 64 + SPRITE_OFFSET_Y - 16 - cos(rotation) * 32;
}
