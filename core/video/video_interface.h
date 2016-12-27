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

#ifndef video_interface_h
#define video_interface_h

#include <stdio.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define NUM_CHARACTER_BANKS 2
#define NUM_CHARACTERS 256
#define NUM_COLORS 32
#define NUM_PLANES 2
#define PLANE_COLUMNS 32
#define PLANE_ROWS 32
#define WINDOW_COLUMNS 16
#define WINDOW_ROWS 16
#define NUM_SPRITES 64
#define SPRITE_OFFSET_X 8
#define SPRITE_OFFSET_Y 8


typedef struct {
    uint16_t data[8];
} Character;

typedef struct {
    Character characters[NUM_CHARACTERS];
} CharacterBank;

typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t character;
    // attributes (8 bit)
    uint8_t attr_palette:3;
    uint8_t attr_bank:1;
    uint8_t attr_flipX:1;
    uint8_t attr_flipY:1;
    uint8_t attr_priority:1;
} Sprite;

typedef struct {
    uint8_t character;
    // attributes (8 bit)
    uint8_t attr_palette:3;
    uint8_t attr_bank:1;
    uint8_t attr_flipX:1;
    uint8_t attr_flipY:1;
    uint8_t attr_priority:1;
} Cell;

typedef struct {
    uint8_t scrollX;
    uint8_t scrollY;
    Cell cells[PLANE_ROWS][PLANE_COLUMNS];
} Plane;

typedef struct {
    Cell cells[WINDOW_ROWS][WINDOW_COLUMNS];
} Window;

typedef struct {
    CharacterBank characterBanks[NUM_CHARACTER_BANKS];
    Plane planes[NUM_PLANES];
    Window window;
    Sprite sprites[NUM_SPRITES];
    uint8_t colors[NUM_COLORS];
} VideoInterface;


void LRC_renderScreen(VideoInterface *vi, uint8_t *outputRGB);

#endif /* video_interface_h */
