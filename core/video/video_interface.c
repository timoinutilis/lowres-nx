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

#include "video_interface.h"
#include "character_rom.h"

int LRC_getCharacterPixel(Character *character, int x, int y)
{
    return (character->data[y] >> ((7 - x) << 1)) & 0x03;
}

void LRC_renderPlane(VideoInterface *vi, int index, int priority, int y, uint8_t *scanlineBuffer)
{
    Plane *plane = &vi->planes[index];
    int planeY = y + plane->scrollY;
    int row = (planeY >> 3) & 31;
    int cellY = planeY & 7;
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int planeX = x + plane->scrollX;
        int column = (planeX >> 3) & 31;
        int cellX = planeX & 7;
        Cell *cell = &plane->cells[row][column];
//        Character *character = &vi->characterBanks[0].characters[cell->character];
        Character *character = (Character *)CharacterRom[cell->character];
        int pixel = LRC_getCharacterPixel(character, cellX, cellY);
        if (pixel != 0)
        {
            *scanlineBuffer = pixel;
        }
        scanlineBuffer++;
    }
}

void LRC_renderWindow(VideoInterface *vi, int priority, int y, uint8_t *scanlineBuffer)
{
    Window *window = &vi->window;
    int row = y >> 3;
    int cellY = y & 7;
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int column = x >> 3;
        int cellX = x & 7;
        Cell *cell = &window->cells[row][column];
//        Character *character = &vi->characterBanks[0].characters[cell->character];
        Character *character = (Character *)CharacterRom[cell->character];
        int pixel = LRC_getCharacterPixel(character, cellX, cellY);
        if (pixel != 0)
        {
            *scanlineBuffer = pixel;
        }
        scanlineBuffer++;
    }
}

void LRC_renderSprites(VideoInterface *vi, int priority, int y, uint8_t *scanlineBuffer)
{
    
}

void LRC_renderScreen(VideoInterface *vi, uint8_t *outputRGB)
{
    uint8_t scanlineBuffer[SCREEN_WIDTH];
    uint8_t *outputByte = outputRGB;
    
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            scanlineBuffer[x] = 0;
        }
        for (int priority = 0; priority < 1; priority++)
        {
            LRC_renderPlane(vi, 0, priority, y, scanlineBuffer);
            LRC_renderPlane(vi, 1, priority, y, scanlineBuffer);
            LRC_renderSprites(vi, priority, y, scanlineBuffer);
            LRC_renderWindow(vi, priority, y, scanlineBuffer);
        }
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            int color = vi->colors[scanlineBuffer[x]];
            int r = (color >> 4) & 0x03;
            int g = (color >> 2) & 0x03;
            int b = color & 0x03;
            *outputByte++ = r * 0x55;
            *outputByte++ = g * 0x55;
            *outputByte++ = b * 0x55;
        }
    }
}
