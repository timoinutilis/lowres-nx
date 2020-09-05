//
// Copyright 2016-2019 Timo Kloss
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
