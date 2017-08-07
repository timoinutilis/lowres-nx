//
// Copyright 2017 Timo Kloss
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

#include "sprites_lib.h"
#include "core.h"

bool sprlib_checkSingleCollision(struct Core *core, struct Sprite *sprite, struct Sprite *otherSprite)
{
    int ax1 = sprite->x;
    int ay1 = sprite->y;
    int ax2 = otherSprite->x;
    int ay2 = otherSprite->y;
    
    if ((ax1 != 0 || ay1 != 0) && (ax2 != 0 || ay2 != 0))
    {
        int bx1 = ax1 + 8 * (sprite->attr.size + 1);
        int by1 = ay1 + 8 * (sprite->attr.size + 1);
        int bx2 = ax2 + 8 * (otherSprite->attr.size + 1);
        int by2 = ay2 + 8 * (otherSprite->attr.size + 1);
        
        if (bx1 > ax2 && by1 > ay2 && ax1 < bx2 && ay1 < by2)
        {
            return true;
        }
    }
    return false;
}

bool sprlib_checkCollision(struct Core *core, int checkIndex, int firstIndex, int lastIndex)
{
    struct Sprite *sprites = core->machine.spriteRegisters.sprites;
    struct Sprite *sprite = &sprites[checkIndex];
    
    for (int i = firstIndex; i <= lastIndex; i++)
    {
        if (i != checkIndex)
        {
            if (sprlib_checkSingleCollision(core, sprite, &sprites[i]))
            {
                core->interpreter.spritesLib.lastHit = i;
                return true;
            }
        }
    }
    return false;
}
