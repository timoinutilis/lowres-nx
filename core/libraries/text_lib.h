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

#ifndef text_lib_h
#define text_lib_h

#include <stdio.h>
#include "video_interface.h"

struct LowResCore;

struct TextLib {
    uint8_t characterOffset;
    union CharacterAttributes charAttr;
    uint8_t areaX;
    uint8_t areaY;
    uint8_t areaWidth;
    uint8_t areaHeight;
    uint8_t cursorX;
    uint8_t cursorY;
};

void LRC_printText(struct LowResCore *core, const char *text);
void LRC_writeText(struct LowResCore *core, const char *text, int x, int y);
void LRC_writeNumber(struct LowResCore *core, int number, int digits, int x, int y);

#endif /* text_lib_h */
