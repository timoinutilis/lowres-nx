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

#ifndef interpreter_utils_h
#define interpreter_utils_h

#include <stdio.h>
#include <stdbool.h>
#include "value.h"
#include "video_chip.h"
#include "audio_chip.h"

struct Core;

struct SimpleAttributes
{
    int pal;
    int flipX;
    int flipY;
    int prio;
    int size;
};

enum ErrorCode itp_evaluateSimpleAttributes(struct Core *core, struct SimpleAttributes *attrs);

struct TypedValue itp_evaluateCharAttributes(struct Core *core, union CharacterAttributes oldAttr);
struct TypedValue itp_evaluateDisplayAttributes(struct Core *core, union DisplayAttributes oldAttr);
struct TypedValue itp_evaluateLFOAttributes(struct Core *core, union LFOAttributes oldAttr);

#endif /* interpreter_utils_h */
