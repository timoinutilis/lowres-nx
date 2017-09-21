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
#include <assert.h>

void txtlib_init(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    lib->fontCharOffset = 192;
    
    txtlib_clearScreen(core);
    
    core->machine.colorRegisters.colors[1] = (3 << 4) | (3 << 2) | 3;
    core->machine.colorRegisters.colors[2] = (2 << 4) | (2 << 2) | 2;
    core->machine.colorRegisters.colors[3] = (1 << 4) | (1 << 2) | 1;
    
    if (core->interpreter.romIncludesDefaultCharacters)
    {
        struct RomDataEntry *entry0 = core->interpreter.romDataEntries;
        memcpy(core->machine.videoRam.characters, &core->machine.cartridgeRom[entry0->start], entry0->length);
    }
}

struct Plane *txtlib_getCurrentBackground(struct Core *core)
{
    switch (core->interpreter.textLib.bg)
    {
        case 0:
            return &core->machine.videoRam.planeA;
        case 1:
            return &core->machine.videoRam.planeB;
        case 2:
            return &core->overlay.plane;
        default:
            assert(0);
    }
}

struct Plane *txtlib_getWindowBackground(struct Core *core)
{
    switch (core->interpreter.textLib.windowBg)
    {
        case 0:
            return &core->machine.videoRam.planeA;
        case 1:
            return &core->machine.videoRam.planeB;
        case 2:
            return &core->overlay.plane;
        default:
            assert(0);
    }
}

void txtlib_scroll(struct Plane *plane, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY)
{
    for (int y = fromY; y <= toY; y++)
    {
        for (int x = fromX; x <= toX; x++)
        {
            plane->cells[y][x] = plane->cells[(y - deltaY) & 0x1F][(x - deltaX) & 0x1F];
        }
    }
}

void txtlib_scrollWindowIfNeeded(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = txtlib_getWindowBackground(core);
    
    if (lib->cursorY >= lib->windowHeight)
    {
        // scroll
        txtlib_scroll(plane, lib->windowX, lib->windowY, lib->windowX + lib->windowWidth - 1, lib->windowY + lib->windowHeight - 1, 0, -1);
        
        // clear bottom line
        int py = lib->windowY + lib->windowHeight - 1;
        for (int x = 0; x < lib->windowWidth; x++)
        {
            int px = x + lib->windowX;
            struct Cell *cell = &plane->cells[py][px];
            cell->character = lib->fontCharOffset; // space
            cell->attr = lib->fontCharAttr;
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
        txtlib_scrollWindowIfNeeded(core);
        
        if (*letter >= 32)
        {
            struct Cell *cell = &plane->cells[lib->cursorY + lib->windowY][lib->cursorX + lib->windowX];
            cell->attr.value = lib->fontCharAttr.value;
            cell->character = lib->fontCharOffset + (*letter - 32);
        
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
    cell->attr.value = lib->fontCharAttr.value;
    cell->character = lib->fontCharOffset;
    
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
    cell->attr.value = lib->fontCharAttr.value;
    cell->character = lib->fontCharOffset;
    
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
            cell->attr.value = lib->fontCharAttr.value;
            cell->character = lib->fontCharOffset + (*letter - 32);
            
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
        cell->attr.value = lib->fontCharAttr.value;
        cell->character = lib->fontCharOffset + ((number / div) % 10 + 16);
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
    
    core->machine.ioRegisters.attr.keyboardEnabled = 1;
    core->delegate->controlsDidChange(core->delegate->context);
    
    txtlib_scrollWindowIfNeeded(core);
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
            cell->attr.value = lib->fontCharAttr.value;
            cell->character = lib->fontCharOffset;
            
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
                
                txtlib_scrollWindowIfNeeded(core);
            }
        }
        lib->blink = 0;
        core->machine.ioRegisters.key = 0;
    }
    if (!done)
    {
        struct Cell *cell = &plane->cells[lib->cursorY + lib->windowY][lib->cursorX + lib->windowX];
        cell->attr.value = lib->fontCharAttr.value;
        cell->character = lib->fontCharOffset + (lib->blink++ < 15 ? 63 : 0);
        
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
            cell->character = lib->fontCharOffset;
            cell->attr = lib->fontCharAttr;
        }
    }
}

void txtlib_clearScreen(struct Core *core)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct VideoRegisters *reg = &core->machine.videoRegisters;
    
    memset(&core->machine.videoRam.planeA, 0, sizeof(struct Plane));
    memset(&core->machine.videoRam.planeB, 0, sizeof(struct Plane));
    
    reg->scrollAX = 0;
    reg->scrollAY = 0;
    reg->scrollBX = 0;
    reg->scrollBY = 0;
    reg->attr.spritesEnabled = 1;
    reg->attr.planeAEnabled = 1;
    reg->attr.planeBEnabled = 1;
    
    lib->windowX = 0;
    lib->windowY = 0;
    lib->windowWidth = 20;
    lib->windowHeight = 16;
    lib->cursorX = 0;
    lib->cursorY = 0;
    lib->bg = 0;
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

struct Cell *txtlib_getCell(struct Core *core, int x, int y)
{
    struct Plane *plane = txtlib_getCurrentBackground(core);
    return &plane->cells[y][x];
}

void txtlib_setCell(struct Core *core, int x, int y)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = txtlib_getCurrentBackground(core);
    struct Cell *cell = &plane->cells[y][x];
    cell->character = lib->cellChar;
    cell->attr = lib->cellCharAttr;
}

void txtlib_setCells(struct Core *core, int fromX, int fromY, int toX, int toY)
{
    struct TextLib *lib = &core->interpreter.textLib;
    struct Plane *plane = txtlib_getCurrentBackground(core);
    for (int y = fromY; y <= toY; y++)
    {
        for (int x = fromX; x <= toX; x++)
        {
            struct Cell *cell = &plane->cells[y][x];
            cell->character = lib->cellChar;
            cell->attr = lib->cellCharAttr;
        }
    }
}

void txtlib_scrollBackground(struct Core *core, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY)
{
    struct Plane *plane = txtlib_getCurrentBackground(core);
    txtlib_scroll(plane, fromX, fromY, toX, toY, deltaX, deltaY);
}

void txtlib_copyBackground(struct Core *core, int srcX, int srcY, int width, int height, int dstX, int dstY)
{
    struct Plane *plane = txtlib_getCurrentBackground(core);
    struct TextLib *lib = &core->interpreter.textLib;
    
    for (int y = 0; y < height; y++)
    {
        int py = dstY + y;
        int addr = lib->sourceAddress + ((srcY + y) * lib->sourceWidth + srcX) * 2;
        for (int x = 0; x < width; x++)
        {
            int px = dstX + x;
            struct Cell *cell = &plane->cells[py & 0x1F][px & 0x1F];
            cell->character = machine_peek(core, addr++);
            cell->attr.value = machine_peek(core, addr++);
        }
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
