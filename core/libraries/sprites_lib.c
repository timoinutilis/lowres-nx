//
// Copyright 2017 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "sprites_lib.h"
#include "core.h"
#include <stdint.h>

bool sprlib_isSpriteOnScreen(struct Sprite *sprite)
{
    int size = (sprite->attr.size + 1) << 3;
    return (   sprite->x < SCREEN_WIDTH + SPRITE_OFFSET_X
            && sprite->y < SCREEN_HEIGHT + SPRITE_OFFSET_Y
            && sprite->x + size > SPRITE_OFFSET_X
            && sprite->y + size > SPRITE_OFFSET_Y);
}

bool sprlib_checkSingleCollision(struct SpritesLib *lib, struct Sprite *sprite, struct Sprite *otherSprite)
{
    if (sprlib_isSpriteOnScreen(otherSprite))
    {
        int ax1 = sprite->x;
        int ay1 = sprite->y;
        
        int ax2 = otherSprite->x;
        int ay2 = otherSprite->y;
        
        int s1 = (sprite->attr.size + 1) << 3;
        int s2 = (otherSprite->attr.size + 1) << 3;
        
        int bx1 = ax1 + s1;
        int by1 = ay1 + s1;
        int bx2 = ax2 + s2;
        int by2 = ay2 + s2;
        
        // rectangle check
        if (bx1 > ax2 && by1 > ay2 && ax1 < bx2 && ay1 < by2)
        {
            // pixel exact check
            int diffX = ax2 - ax1;
            int diffY = ay2 - ay1;
            
            struct Character *characters = lib->core->machine->videoRam.characters;
            int c1 = sprite->character;
            int c2 = otherSprite->character;
            
            for (int line = 0; line < s1; line++)
            {
                if (line - diffY >= 0 && line - diffY < s2)
                {
                    int line1 = sprite->attr.flipY ? (s1 - line - 1) : line;
                    int line2 = otherSprite->attr.flipY ? (s2 - (line - diffY) - 1) : (line - diffY);
                    bool flx1 = sprite->attr.flipX;
                    bool flx2 = otherSprite->attr.flipX;
                    
                    uint32_t source1 = 0;
                    int chLine1 = line1 & 7;
                    int rc1 = c1 + (line1 >> 3 << 4);
                    for (int i = 0; i <= sprite->attr.size; i++)
                    {
                        uint8_t *data = characters[flx1 ? (rc1 + sprite->attr.size - i) : (rc1 + i)].data;
                        uint32_t val = (data[chLine1] | data[chLine1 + 8]);
                        if (flx1)
                        {
                            // reverse bits
                            val = (((val * 0x0802LU & 0x22110LU) | (val * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0xFF;
                        }
                        source1 |= val << (24 - (i << 3));
                    }
                    
                    uint32_t source2 = 0;
                    int chLine2 = line2 & 7;
                    int rc2 = c2 + (line2 >> 3 << 4);
                    for (int i = 0; i <= otherSprite->attr.size; i++)
                    {
                        uint8_t *data = characters[flx2 ? (rc2 + otherSprite->attr.size - i) : (rc2 + i)].data;
                        uint32_t val = (data[chLine2] | data[chLine2 + 8]);
                        if (flx2)
                        {
                            // reverse bits
                            val = (((val * 0x0802LU & 0x22110LU) | (val * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0xFF;
                        }
                        
                        int shift = (24 - (i << 3) - diffX);
                        if (shift >= 0 && shift < 32)
                        {
                            source2 |= val << shift;
                        }
                        else if (shift > -32 && shift < 0)
                        {
                            source2 |= val >> -shift;
                        }
                    }
                    
                    if (source1 & source2)
                    {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

bool sprlib_checkCollision(struct SpritesLib *lib, int checkIndex, int firstIndex, int lastIndex)
{
    struct Sprite *sprites = lib->core->machine->spriteRegisters.sprites;
    struct Sprite *sprite = &sprites[checkIndex];
    
    if (sprlib_isSpriteOnScreen(sprite))
    {
        for (int i = firstIndex; i <= lastIndex; i++)
        {
            if (i != checkIndex)
            {
                if (sprlib_checkSingleCollision(lib, sprite, &sprites[i]))
                {
                    lib->lastHit = i;
                    return true;
                }
            }
        }
    }
    return false;
}
