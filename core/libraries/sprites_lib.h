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

#ifndef sprites_lib_h
#define sprites_lib_h

#include <stdio.h>
#include <stdbool.h>

struct Core;
struct Sprite;

struct SpritesLib {
    struct Core *core;
    int lastHit;
};

bool sprlib_isSpriteOnScreen(struct Sprite *sprite);
bool sprlib_checkCollision(struct SpritesLib *lib, int checkIndex, int firstIndex, int lastIndex);

#endif /* sprites_lib_h */
