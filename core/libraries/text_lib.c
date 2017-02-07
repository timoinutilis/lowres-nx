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

#include "text_lib.h"
#include "system_ram.h"

void LRC_printText(struct Machine *machine, const char *text)
{
    struct TextLib *lib = &((struct SystemRam *)machine->workingRam)->textLib;
    struct Plane *plane = &machine->videoRam.planeB;
    const char *letter = text;
    while (*letter)
    {
        if (lib->cursorY >= lib->areaHeight)
        {
            // scroll
            for (int y = 0; y < lib->areaHeight - 1; y++)
            {
                int py = y + lib->areaY;
                for (int x = 0; x < lib->areaWidth; x++)
                {
                    int px = x + lib->areaX;
                    plane->cells[py][px] = plane->cells[py+1][px];
                }
            }
            int py = lib->areaY + lib->areaHeight - 1;
            for (int x = 0; x < lib->areaWidth; x++)
            {
                int px = x + lib->areaX;
                struct Cell *cell = &plane->cells[py][px];
                cell->character = lib->characterOffset; // space
                cell->attr.value = lib->charAttr.value;
            }
            
            lib->cursorY = lib->areaHeight - 1;
        }
        
        if (*letter >= 32)
        {
            struct Cell *cell = &plane->cells[lib->cursorY + lib->areaY][lib->cursorX + lib->areaX];
            cell->attr.value = lib->charAttr.value;
            cell->character = lib->characterOffset + (*letter - 32);
        
            lib->cursorX++;
        }
        else if (*letter == '\n')
        {
            lib->cursorX = 0;
            lib->cursorY++;
        }
        
        if (lib->cursorX >= lib->areaWidth)
        {
            lib->cursorX = 0;
            lib->cursorY++;
        }
        
        letter++;
    }
}

void LRC_writeText(struct Machine *machine, const char *text, int x, int y)
{
    struct TextLib *lib = &((struct SystemRam *)machine->workingRam)->textLib;
    struct Plane *plane = &machine->videoRam.planeB;
    const char *letter = text;
    while (*letter)
    {
        if (*letter >= 32)
        {
            struct Cell *cell = &plane->cells[y][x];
            cell->attr.value = lib->charAttr.value;
            cell->character = lib->characterOffset + (*letter - 32);
            
            x++;
        }
        letter++;
    }
}

void LRC_writeNumber(struct Machine *machine, int number, int digits, int x, int y)
{
    struct TextLib *lib = &((struct SystemRam *)machine->workingRam)->textLib;
    struct Plane *plane = &machine->videoRam.planeB;
    
    x += digits;
    int div = 1;
    for (int i = 0; i < digits; i++)
    {
        x--;
        struct Cell *cell = &plane->cells[y][x];
        cell->attr.value = lib->charAttr.value;
        cell->character = lib->characterOffset + ((number / div) % 10 + 16);
        div *= 10;
    }
}
