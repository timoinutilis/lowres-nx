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
#include <stdbool.h>
#include "video_interface.h"

#define INPUT_BUFFER_SIZE 256

struct LowResCore;

struct TextLib {
    int characterOffset;
    union CharacterAttributes charAttr;
    int areaX;
    int areaY;
    int areaWidth;
    int areaHeight;
    int cursorX;
    int cursorY;
    char inputBuffer[INPUT_BUFFER_SIZE];
    int inputLength;
    int blink;
};

void LRC_printText(struct LowResCore *core, const char *text);
bool LRC_deleteBackward(struct LowResCore *core);
void LRC_writeText(struct LowResCore *core, const char *text, int x, int y);
void LRC_writeNumber(struct LowResCore *core, int number, int digits, int x, int y);
void LRC_inputTextBegin(struct LowResCore *core);
bool LRC_inputTextUpdate(struct LowResCore *core);

#endif /* text_lib_h */
