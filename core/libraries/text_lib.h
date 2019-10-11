//
// Copyright 2016-2019 Timo Kloss
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

#ifndef text_lib_h
#define text_lib_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "video_chip.h"

#define INPUT_BUFFER_SIZE 256
#define OVERLAY_BG 2

struct Core;

struct TextLib {
    struct Core *core;
    union CharacterAttributes charAttr;
    int fontCharOffset;
    int windowX;
    int windowY;
    int windowWidth;
    int windowHeight;
    int windowBg;
    int cursorX;
    int cursorY;
    int bg;
    int sourceAddress;
    int sourceWidth;
    int sourceHeight;
    char inputBuffer[INPUT_BUFFER_SIZE];
    int inputLength;
    int blink;
};

void txtlib_printText(struct TextLib *lib, const char *text);
bool txtlib_deleteBackward(struct TextLib *lib);
void txtlib_writeText(struct TextLib *lib, const char *text, int x, int y);
void txtlib_writeNumber(struct TextLib *lib, int number, int digits, int x, int y);
void txtlib_inputBegin(struct TextLib *lib);
bool txtlib_inputUpdate(struct TextLib *lib);
void txtlib_clearWindow(struct TextLib *lib);
void txtlib_clearScreen(struct TextLib *lib);
void txtlib_clearBackground(struct TextLib *lib, int bg);
struct Cell *txtlib_getCell(struct TextLib *lib, int x, int y);
void txtlib_setCell(struct TextLib *lib, int x, int y, int character);
void txtlib_setCells(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int character);
void txtlib_setCellsAttr(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int pal, int flipX, int flipY, int prio);
void txtlib_scrollBackground(struct TextLib *lib, int fromX, int fromY, int toX, int toY, int deltaX, int deltaY);
void txtlib_copyBackground(struct TextLib *lib, int srcX, int srcY, int width, int height, int dstX, int dstY);
int txtlib_getSourceCell(struct TextLib *lib, int x, int y, bool getAttrs);
bool txtlib_setSourceCell(struct TextLib *lib, int x, int y, int character);

void txtlib_itobin(char *buffer, size_t buffersize, size_t width, int value);

#endif /* text_lib_h */
