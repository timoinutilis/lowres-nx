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
#include <string.h>

int LRC_getCharacterPixel(Character *character, int x, int y)
{
    return (character->data[y] >> ((7 - x) << 1)) & 0x03;
}

Character *LRC_getCharacter(VideoInterface *vi, int bank, int characterIndex)
{
    Character *character;
    if (bank)
    {
        character = (Character *)CharacterRom[characterIndex];
    }
    else
    {
        character = &vi->characters[characterIndex];
    }
    return character;
}

void LRC_renderPlane(VideoInterface *vi, int index, int y, uint8_t *scanlineBuffer)
{
    Plane *plane = &vi->planes[index];
    int planeY = y + plane->scrollY;
    int row = (planeY >> 3) & 31;
    int cellY = planeY & 7;
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int planeX = x + plane->scrollX;
        int column = (planeX >> 3) & 31;
        Cell *cell = &plane->cells[row][column];
        if (cell->attr_priority >= (*scanlineBuffer >> 7))
        {
            int cellX = planeX & 7;
            Character *character = LRC_getCharacter(vi, cell->attr_bank, cell->character);
            int pixel = LRC_getCharacterPixel(character, cell->attr_flipX ? (7 - cellX) : cellX, cell->attr_flipY ? (7 - cellY) : cellY);
            if (pixel)
            {
                *scanlineBuffer = pixel | (cell->attr_palette << 2) | (cell->attr_priority << 7);
            }
        }
        scanlineBuffer++;
    }
}

void LRC_renderWindow(VideoInterface *vi, int y, uint8_t *scanlineBuffer)
{
    Window *window = &vi->window;
    int row = y >> 3;
    int cellY = y & 7;
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int column = x >> 3;
        int cellX = x & 7;
        Cell *cell = &window->cells[row][column];
        if (cell->attr_priority >= (*scanlineBuffer >> 7))
        {
            Character *character = LRC_getCharacter(vi, cell->attr_bank, cell->character);
            int pixel = LRC_getCharacterPixel(character, cell->attr_flipX ? (7 - cellX) : cellX, cell->attr_flipY ? (7 - cellY) : cellY);
            if (pixel)
            {
                *scanlineBuffer = pixel | (cell->attr_palette << 2) | (cell->attr_priority << 7);
            }
        }
        scanlineBuffer++;
    }
}

void LRC_renderSprites(VideoInterface *vi, int y, uint8_t *scanlineBuffer, uint8_t *scanlineSpriteBuffer)
{
    for (int i = 0; i < NUM_SPRITES; i++)
    {
        Sprite *sprite = &vi->sprites[i];
        if (sprite->x != 0 || sprite->y != 0)
        {
            int spriteY = y - sprite->y + SPRITE_OFFSET_Y;
            int height = (sprite->attr_height + 1) << 3;
            if (spriteY >= 0 && spriteY < height)
            {
                if (sprite->attr_flipY)
                {
                    spriteY = height - spriteY - 1;
                }
                int charIndex = sprite->character + ((spriteY >> 3) << 4);
                if (sprite->attr_flipX)
                {
                    charIndex += sprite->attr_width;
                }
                Character *character = LRC_getCharacter(vi, sprite->attr_bank, charIndex);
                int width = (sprite->attr_width + 1) << 3;
                int minX = sprite->x - SPRITE_OFFSET_X;
                int maxX = minX + width;
                if (minX < 0) minX = 0;
                if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;
                uint8_t *buffer = &scanlineSpriteBuffer[minX];
                int spriteX = minX - sprite->x + SPRITE_OFFSET_X;
                if (sprite->attr_flipX)
                {
                    spriteX = width - spriteX - 1;
                }
                for (int x = minX; x < maxX; x++)
                {
                    int pixel = LRC_getCharacterPixel(character, spriteX & 0x07, spriteY & 0x07);
                    if (pixel)
                    {
                        *buffer = pixel | (sprite->attr_palette << 2) | (sprite->attr_priority << 7);
                    }
                    buffer++;
                    if (sprite->attr_flipX)
                    {
                        if (!(spriteX & 0x07))
                        {
                            character--;
                        }
                        spriteX--;
                    }
                    else
                    {
                        spriteX++;
                        if (!(spriteX & 0x07))
                        {
                            character++;
                        }
                    }
                }
            }
        }
    }
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int pixel = *scanlineSpriteBuffer;
        if (pixel && (pixel >> 7) >= (*scanlineBuffer >> 7))
        {
            *scanlineBuffer = pixel;
        }
        scanlineSpriteBuffer++;
        scanlineBuffer++;
    }
}

void LRC_renderScreen(VideoInterface *vi, uint8_t *outputRGB)
{
    uint8_t scanlineBuffer[SCREEN_WIDTH];
    uint8_t scanlineSpriteBuffer[SCREEN_WIDTH];
    uint8_t *outputByte = outputRGB;
    
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        memset(scanlineBuffer, 0, sizeof(scanlineBuffer));
        memset(scanlineSpriteBuffer, 0, sizeof(scanlineSpriteBuffer));
        LRC_renderPlane(vi, 0, y, scanlineBuffer);
        LRC_renderPlane(vi, 1, y, scanlineBuffer);
        LRC_renderSprites(vi, y, scanlineBuffer, scanlineSpriteBuffer);
        LRC_renderWindow(vi, y, scanlineBuffer);
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            int color = vi->colors[scanlineBuffer[x] & 0x7F];
            int r = (color >> 4) & 0x03;
            int g = (color >> 2) & 0x03;
            int b = color & 0x03;
            *outputByte++ = r * 0x55;
            *outputByte++ = g * 0x55;
            *outputByte++ = b * 0x55;
        }
    }
}
