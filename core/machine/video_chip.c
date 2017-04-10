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
#include "character_rom.h"
#include "core.h"
#include <string.h>

int video_getCharacterPixel(struct Character *character, int x, int y)
{
    if (x < 4)
    {
        return (character->data[y << 1] >> ((3 - x) << 1)) & 0x03;
    }
    else
    {
        return (character->data[(y << 1) + 1] >> ((7 - x) << 1)) & 0x03;
    }
}

struct Character *video_getCharacter(struct VideoRam *ram, int bank, int characterIndex)
{
    struct CharacterBank *characterBank;
    if (bank)
    {
        characterBank = (struct CharacterBank *)CharacterRom;
    }
    else
    {
        characterBank = &ram->characterBank;
    }
    return &characterBank->characters[characterIndex];
}

void video_renderPlane(struct VideoRegisters *reg, struct VideoRam *ram, struct Plane *plane, int y, int scrollX, int scrollY, uint8_t *scanlineBuffer)
{
    int planeY = y + scrollY;
    int row = (planeY >> 3) & 31;
    int cellY = planeY & 7;
    for (int x = 0; x < SCREEN_WIDTH; x++)
    {
        int planeX = x + scrollX;
        int column = (planeX >> 3) & 31;
        struct Cell *cell = &plane->cells[row][column];
        if (cell->attr.priority >= (*scanlineBuffer >> 7))
        {
            int cellX = planeX & 7;
            struct Character *character = video_getCharacter(ram, cell->attr.bank, cell->character);
            int pixel = video_getCharacterPixel(character, cell->attr.flipX ? (7 - cellX) : cellX, cell->attr.flipY ? (7 - cellY) : cellY);
            if (pixel)
            {
                *scanlineBuffer = pixel | (cell->attr.palette << 2) | (cell->attr.priority << 7);
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
            int height = (sprite->attr2.height + 1) << 3;
            if (spriteY >= 0 && spriteY < height)
            {
                if (sprite->attr1.flipY)
                {
                    spriteY = height - spriteY - 1;
                }
                int charIndex = sprite->character + ((spriteY >> 3) << 4);
                if (sprite->attr1.flipX)
                {
                    charIndex += sprite->attr2.width;
                }
                struct Character *character = video_getCharacter(ram, sprite->attr1.bank, charIndex);
                int width = (sprite->attr2.width + 1) << 3;
                int minX = sprite->x - SPRITE_OFFSET_X;
                int maxX = minX + width;
                if (minX < 0) minX = 0;
                if (maxX > SCREEN_WIDTH) maxX = SCREEN_WIDTH;
                uint8_t *buffer = &scanlineSpriteBuffer[minX];
                int spriteX = minX - sprite->x + SPRITE_OFFSET_X;
                if (sprite->attr1.flipX)
                {
                    spriteX = width - spriteX - 1;
                }
                for (int x = minX; x < maxX; x++)
                {
                    int pixel = video_getCharacterPixel(character, spriteX & 0x07, spriteY & 0x07);
                    if (pixel)
                    {
                        *buffer = pixel | (sprite->attr1.palette << 2) | (sprite->attr1.priority << 7);
                    }
                    buffer++;
                    if (sprite->attr1.flipX)
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

void video_renderScreen(struct Core *core, uint8_t *outputRGB, int bytesPerLine)
{
    uint8_t scanlineBuffer[SCREEN_WIDTH];
    uint8_t scanlineSpriteBuffer[SCREEN_WIDTH];
    uint8_t *outputByte = outputRGB;
    
    struct VideoRam *ram = &core->machine.videoRam;
    struct VideoRegisters *reg = &core->machine.videoRegisters;
    struct SpriteRegisters *sreg = &core->machine.spriteRegisters;
    struct ColorRegisters *creg = &core->machine.colorRegisters;
    for (int y = 0; y < SCREEN_HEIGHT; y++)
    {
        reg->rasterLine = y;
        core_rasterUpdate(core);
        memset(scanlineBuffer, 0, sizeof(scanlineBuffer));
        memset(scanlineSpriteBuffer, 0, sizeof(scanlineSpriteBuffer));
        video_renderPlane(reg, ram, &ram->planeB, y, reg->scrollBX, reg->scrollBY, scanlineBuffer);
        video_renderPlane(reg, ram, &ram->planeA, y, reg->scrollAX, reg->scrollAY, scanlineBuffer);
        video_renderSprites(sreg, ram, y, scanlineBuffer, scanlineSpriteBuffer);
        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            int color = creg->colors[scanlineBuffer[x] & 0x7F];
            int r = (color >> 4) & 0x03;
            int g = (color >> 2) & 0x03;
            int b = color & 0x03;
            *outputByte++ = r * 0x55;
            *outputByte++ = g * 0x55;
            *outputByte++ = b * 0x55;
        }
        outputByte += (bytesPerLine - SCREEN_WIDTH*3);
    }
}
