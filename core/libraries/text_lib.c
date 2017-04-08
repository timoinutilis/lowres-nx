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
#include "lowres_core.h"

void LRC_scrollIfNeeded(struct LowResCore *core)
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
    }
}

void LRC_printText(struct LowResCore *core, const char *text)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = &core->machine.videoRam.planeB;
    const char *letter = text;
    while (*letter)
    {
        LRC_scrollIfNeeded(core);
        
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

bool LRC_deleteBackward(struct LowResCore *core)
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

void LRC_writeText(struct LowResCore *core, const char *text, int x, int y)
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

void LRC_writeNumber(struct LowResCore *core, int number, int digits, int x, int y)
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

void LRC_inputTextBegin(struct LowResCore *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    lib->inputBuffer[0] = 0;
    lib->inputLength = 0;
    lib->blink = 0;
    core->machine.ioRegisters.key = 0;
    
    LRC_scrollIfNeeded(core);
}

bool LRC_inputTextUpdate(struct LowResCore *core)
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
                if (LRC_deleteBackward(core))
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
            
            LRC_printText(core, "\n");
            done = true;
        }
        else
        {
            if (lib->inputLength < INPUT_BUFFER_SIZE - 2)
            {
                char text[2] = {key, 0};
                LRC_printText(core, text);
                lib->inputBuffer[lib->inputLength++] = key;
                lib->inputBuffer[lib->inputLength] = 0;
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
