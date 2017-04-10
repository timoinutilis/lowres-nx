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

#include "text_lib.h"
#include "core.h"

void txtlib_scrollIfNeeded(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
    
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
        core->interpreter.exitEvaluation = true;
    }
}

void txtlib_printText(struct Core *core, const char *text)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
    const char *letter = text;
    while (*letter)
    {
        txtlib_scrollIfNeeded(core);
        
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

bool txtlib_deleteBackward(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
    
    // clear cursor
    struct Cell *cell = &plane->cells[lib->cursorY + lib->areaY][lib->cursorX + lib->areaX];
    cell->attr.value = lib->charAttr.value;
    cell->character = lib->characterOffset;
    
    // move back cursor
    if (lib->cursorX > 0)
    {
        lib->cursorX--;
    }
    else if (lib->cursorY > 0)
    {
        lib->cursorX = lib->areaX + lib->areaWidth - 1;
        lib->cursorY--;
    }
    else
    {
        return false;
    }
    
    // clear cell
    cell = &plane->cells[lib->cursorY + lib->areaY][lib->cursorX + lib->areaX];
    cell->attr.value = lib->charAttr.value;
    cell->character = lib->characterOffset;
    
    return true;
}

void txtlib_writeText(struct Core *core, const char *text, int x, int y)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
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

void txtlib_writeNumber(struct Core *core, int number, int digits, int x, int y)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
    
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

void txtlib_inputBegin(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    lib->inputBuffer[0] = 0;
    lib->inputLength = 0;
    lib->blink = 0;
    core->machine.ioRegisters.key = 0;
    
    txtlib_scrollIfNeeded(core);
}

bool txtlib_inputUpdate(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
    
    char key = core->machine.ioRegisters.key;
    bool done = false;
    if (key)
    {
        if (key == '\b')
        {
            if (lib->inputLength > 0)
            {
                if (txtlib_deleteBackward(core))
                {
                    lib->inputBuffer[--lib->inputLength] = 0;
                }
            }
        }
        else if (key == '\n')
        {
            // clear cursor
            struct Cell *cell = &plane->cells[lib->cursorY + lib->areaY][lib->cursorX + lib->areaX];
            cell->attr.value = lib->charAttr.value;
            cell->character = lib->characterOffset;
            
            txtlib_printText(core, "\n");
            done = true;
        }
        else
        {
            if (lib->inputLength < INPUT_BUFFER_SIZE - 2)
            {
                char text[2] = {key, 0};
                txtlib_printText(core, text);
                lib->inputBuffer[lib->inputLength++] = key;
                lib->inputBuffer[lib->inputLength] = 0;
                
                txtlib_scrollIfNeeded(core);
            }
        }
        lib->blink = 0;
        core->machine.ioRegisters.key = 0;
    }
    if (!done)
    {
        struct Cell *cell = &plane->cells[lib->cursorY + lib->areaY][lib->cursorX + lib->areaX];
        cell->attr.value = lib->charAttr.value;
        cell->character = lib->characterOffset + (lib->blink++ < 15 ? 63 : 0);
        
        if (lib->blink == 30)
        {
            lib->blink = 0;
        }
    }
    return done;
}

void txtlib_clear(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
    
    lib->cursorX = 0;
    lib->cursorY = 0;
    for (int y = 0; y < lib->areaHeight; y++)
    {
        int py = y + lib->areaY;
        for (int x = 0; x < lib->areaWidth; x++)
        {
            int px = x + lib->areaX;
            struct Cell *cell = &plane->cells[py][px];
            cell->character = lib->characterOffset;
            cell->attr = lib->charAttr;
        }
    }
}
