//
// Copyright 2017-2018 Timo Kloss
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

#ifndef overlay_h
#define overlay_h

#include <stdio.h>
#include <stdbool.h>
#include "video_chip.h"
#include "overlay_data.h"
#include "text_lib.h"

struct Core;

struct Overlay {
    struct Plane plane;
    struct TextLib textLib;
    int timer;
    int messageTimer;
};

void overlay_init(struct Core *core);
void overlay_reset(struct Core *core);
void overlay_updateState(struct Core *core);
void overlay_message(struct Core *core, const char *message);
void overlay_draw(struct Core *core, bool ingame);

#endif /* overlay_h */
