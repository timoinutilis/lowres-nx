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
#include <string.h>

void txtlib_init(struct Core *core)
{
    txtlib_clearScreen(core);
    core->interpreter.textLib.charAttr.bank = 1;
}

struct Plane *txtlib_getCurrentBackground(struct Core *core)
{
    return (core->interpreter.textLib.bg == 0) ? &core->machine.videoRam.planeA : &core->machine.videoRam.planeB;
}

struct Plane *txtlib_getWindowBackground(struct Core *core)
{
    return (core->interpreter.textLib.windowBg == 0) ? &core->machine.videoRam.planeA : &core->machine.videoRam.planeB;
}

void txtlib_scrollIfNeeded(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = txtlib_getWindowBackground(core);
    
    if (lib->cursorY >= lib->windowHeight)
    {
        // scroll
        for (int y = 0; y < lib->windowHeight - 1; y++)
        {
            int py = y + lib->windowY;
            for (int x = 0; x < lib->windowWidth; x++)
            {
                int px = x + lib->windowX;
                plane->cells[py][px] = plane->cells[py+1][px];
            }
        }
        int py = lib->windowY + lib->windowHeight - 1;
        for (int x = 0; x < lib->windowWidth; x++)
        {
            int px = x + lib->windowX;
            struct Cell *cell = &plane->cells[py][px];
            cell->character = lib->characterOffset; // space
            cell->attr.value = lib->charAttr.value;
        }
        
        lib->cursorY = lib->windowHeight - 1;
        core->interpreter.exitEvaluation = true;
    }
}

void txtlib_printText(struct Core *core, const char *text)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = txtlib_getWindowBackground(core);
    const char *letter = text;
    while (*letter)
    {
        txtlib_scrollIfNeeded(core);
        
        if (*letter >= 32)
        {
            struct Cell *cell = &plane->cells[lib->cursorY + lib->windowY][lib->cursorX + lib->windowX];
            cell->attr.value = lib->charAttr.value;
            cell->character = lib->characterOffset + (*letter - 32);
        
            lib->cursorX++;
        }
        else if (*letter == '\n')
        {
            lib->cursorX = 0;
            lib->cursorY++;
        }
        
        if (lib->cursorX >= lib->windowWidth)
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
    struct Plane *plane = txtlib_getWindowBackground(core);
    
    // clear cursor
    struct Cell *cell = &plane->cells[lib->cursorY + lib->windowY][lib->cursorX + lib->windowX];
    cell->attr.value = lib->charAttr.value;
    cell->character = lib->characterOffset;
    
    // move back cursor
    if (lib->cursorX > 0)
    {
        lib->cursorX--;
    }
    else if (lib->cursorY > 0)
    {
        lib->cursorX = lib->windowX + lib->windowWidth - 1;
        lib->cursorY--;
    }
    else
    {
        return false;
    }
    
    // clear cell
    cell = &plane->cells[lib->cursorY + lib->windowY][lib->cursorX + lib->windowX];
    cell->attr.value = lib->charAttr.value;
    cell->character = lib->characterOffset;
    
    return true;
}

void txtlib_writeText(struct Core *core, const char *text, int x, int y)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = txtlib_getCurrentBackground(core);
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
    struct Plane *plane = txtlib_getCurrentBackground(core);
    
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
    struct Plane *plane = txtlib_getWindowBackground(core);
    
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
            struct Cell *cell = &plane->cells[lib->cursorY + lib->windowY][lib->cursorX + lib->windowX];
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
        struct Cell *cell = &plane->cells[lib->cursorY + lib->windowY][lib->cursorX + lib->windowX];
        cell->attr.value = lib->charAttr.value;
        cell->character = lib->characterOffset + (lib->blink++ < 15 ? 63 : 0);
        
        if (lib->blink == 30)
        {
            lib->blink = 0;
        }
    }
    return done;
}

void txtlib_clearWindow(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = txtlib_getWindowBackground(core);
    
    lib->cursorX = 0;
    lib->cursorY = 0;
    for (int y = 0; y < lib->windowHeight; y++)
    {
        int py = y + lib->windowY;
        for (int x = 0; x < lib->windowWidth; x++)
        {
            int px = x + lib->windowX;
            struct Cell *cell = &plane->cells[py][px];
            cell->character = lib->characterOffset;
            cell->attr = lib->charAttr;
        }
    }
}

void txtlib_clearScreen(struct Core *core)
{
    memset(&core->machine.videoRam.planeA, 0, sizeof(struct Plane));
    memset(&core->machine.videoRam.planeB, 0, sizeof(struct Plane));
    core->machine.videoRegisters.scrollAX = 0;
    core->machine.videoRegisters.scrollAY = 0;
    core->machine.videoRegisters.scrollBX = 0;
    core->machine.videoRegisters.scrollBY = 0;
    core->interpreter.textLib.windowX = 0;
    core->interpreter.textLib.windowY = 0;
    core->interpreter.textLib.windowWidth = 20;
    core->interpreter.textLib.windowHeight = 16;
    core->interpreter.textLib.cursorX = 0;
    core->interpreter.textLib.cursorY = 0;
    core->interpreter.textLib.bg = 0;
}

void txtlib_clearBackground(struct Core *core, int bg)
{
    if (bg == 0)
    {
        memset(&core->machine.videoRam.planeA, 0, sizeof(struct Plane));
    }
    else if (bg == 1)
    {
        memset(&core->machine.videoRam.planeB, 0, sizeof(struct Plane));
    }
}

void txtlib_itobin(char *buffer, size_t buffersize, size_t width, int value)
{
    if (width < 1)
    {
        width = 1;
    }
    unsigned int mask = 1 << 15;
    int p = 0;
    bool active = false;
    while (mask && p < buffersize - 1)
    {
        if (active || (value & mask) || mask < (1 << width))
        {
            buffer[p++] = (value & mask) ? '1' : '0';
            active = true;
        }
        mask = (mask >> 1) & 0x7FFF;
    }
    buffer[p] = 0;
}
