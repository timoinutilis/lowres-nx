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

#include "video_chip.h"
#include "core.h"
#include <string.h>

#define OVERLAY_FLAG (1<<6)

int video_getCharacterPixel(struct Character *character, int x, int y)
{
    int b0 = (character->data[y] >> (7 - x)) & 0x01;
    int b1 = (character->data[y | 8] >> (7 - x)) & 0x01;
    return b0 | (b1 << 1);
}

void video_renderPlane(struct Character *characters, struct Plane *plane, int sizeMode, int y, int scrollX, int scrollY, int pixelFlag, uint8_t *scanlineBuffer)
{
    int divShift = sizeMode ? 4 : 3;
    int planeY = y + scrollY;
    int row = (planeY >> divShift) & 31;
    int cellY = planeY & 7;
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int planeX = x + scrollX;
        int column = (planeX >> divShift) & 31;
        struct Cell *cell = &plane->cells[row][column];
        if (cell->attr.priority >= (*scanlineBuffer >> 7))
        {
            int cellX = planeX & 7;
            int index = cell->character;
            if (sizeMode)
            {
                index += (cell->attr.flipX ? (planeX >> 3) + 1 : planeX >> 3) & 1;
                index += ((cell->attr.flipY ? (planeY >> 3) + 1 : planeY >> 3) & 1) << 4;
            }
            struct Character *character = &characters[index];
            int pixel = video_getCharacterPixel(character, cell->attr.flipX ? (7 - cellX) : cellX, cell->attr.flipY ? (7 - cellY) : cellY);
            if (pixel)
            {
                *scanlineBuffer = pixel | (cell->attr.palette << 2) | (cell->attr.priority << 7) | pixelFlag;
            }
        }
        scanlineBuffer++;
    }
}

void video_renderSprites(struct SpriteRegisters *reg, struct VideoRam *ram, int y, uint8_t *scanlineBuffer, uint8_t *scanlineSpriteBuffer)
{
    for (int i = NUM_SPRITES - 1; i >= 0; i--)
    {
        struct Sprite *sprite = &reg->sprites[i];
        if (sprite->x != 0 || sprite->y != 0)
        {
            int spriteY = y - sprite->y + SPRITE_OFFSET_Y;
            int size = (sprite->attr.size + 1) << 3;
            if (spriteY >= 0 && spriteY < size)
            {
                if (sprite->attr.flipY)
                {
                    spriteY = size - spriteY - 1;
                }
                int charIndex = sprite->character + ((spriteY >> 3) << 4);
                if (sprite->attr.flipX)
                {
                    charIndex += sprite->attr.size;
                }
                int minX = sprite->x - SPRITE_OFFSET_X;
                int maxX = minX + size;
                if (minX < 0)
                {
                    int skip = -minX >> 3;
                    if (sprite->attr.flipX)
                    {
                        charIndex -= skip;
                    }
                    else
                    {
                        charIndex += skip;
                    }
                }
                if (minX < 0) minX = 0;
                if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;
                uint8_t *buffer = &scanlineSpriteBuffer[minX];
                int spriteX = minX - sprite->x + SPRITE_OFFSET_X;
                if (sprite->attr.flipX)
                {
                    spriteX = size - spriteX - 1;
                }
                struct Character *character = &ram->characters[charIndex];
                for (int x = minX; x < maxX; x++)
                {
                    int pixel = video_getCharacterPixel(character, spriteX & 0x07, spriteY & 0x07);
                    if (pixel)
                    {
                        *buffer = pixel | (sprite->attr.palette << 2) | (sprite->attr.priority << 7);
                    }
                    buffer++;
                    if (sprite->attr.flipX)
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

void video_renderScreen(struct Core *core, uint32_t *outputRGB)
{
    uint8_t scanlineBuffer[SCREEN_WIDTH];
    uint8_t scanlineSpriteBuffer[SCREEN_WIDTH];
    uint32_t *outputPixel = outputRGB;
    
    struct VideoRam *ram = &core->machine->videoRam;
    struct VideoRegisters *reg = &core->machine->videoRegisters;
    struct SpriteRegisters *sreg = &core->machine->spriteRegisters;
    struct ColorRegisters *creg = &core->machine->colorRegisters;
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        reg->rasterLine = y;
        itp_runInterrupt(core, InterruptTypeRaster);
        memset(scanlineBuffer, 0, sizeof(scanlineBuffer));
        
        bool skip = (core->interpreter->interruptOverCycles > 0);
        if (!skip)
        {
            if (reg->attr.planeBEnabled)
            {
                int scrollX = reg->scrollBX | (reg->scrollMSB.bX << 8);
                int scrollY = reg->scrollBY | (reg->scrollMSB.bY << 8);
                video_renderPlane(ram->characters, &ram->planeB, reg->attr.planeBCellSize, y, scrollX, scrollY, 0, scanlineBuffer);
            }
            if (reg->attr.planeAEnabled)
            {
                int scrollX = reg->scrollAX | (reg->scrollMSB.aX << 8);
                int scrollY = reg->scrollAY | (reg->scrollMSB.aY << 8);
                video_renderPlane(ram->characters, &ram->planeA, reg->attr.planeACellSize, y, scrollX, scrollY, 0, scanlineBuffer);
            }
            if (reg->attr.spritesEnabled)
            {
                memset(scanlineSpriteBuffer, 0, sizeof(scanlineSpriteBuffer));
                video_renderSprites(sreg, ram, y, scanlineBuffer, scanlineSpriteBuffer);
            }
        }
        
        // overlay
        video_renderPlane((struct Character *)overlayCharacters, &core->overlay->plane, 0, y, 0, 0, OVERLAY_FLAG, scanlineBuffer);
        
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            int colorIndex = scanlineBuffer[x] & 0x1F;
            int color = (scanlineBuffer[x] & OVERLAY_FLAG) ? overlayColors[colorIndex] : skip ? 0 : creg->colors[colorIndex];
            int r = (color >> 4) & 0x03;
            int g = (color >> 2) & 0x03;
            int b = color & 0x03;
            // add some gray (0x0F) to simulate screen
            *outputPixel = (r * 0x55) | ((g * 0x55) << 8) | ((b * 0x55) << 16) | 0x000F0F0F;
            outputPixel++;
        }
    }
}
