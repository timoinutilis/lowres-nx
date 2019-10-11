//
// Copyright 2017-2018 Timo Kloss
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
