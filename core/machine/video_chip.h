//
// Copyright 2016 Timo Kloss
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

#ifndef video_chip_h
#define video_chip_h

#include <stdio.h>
#include <stdint.h>

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128
#define NUM_CHARACTERS 256
#define NUM_PALETTES 8
#define PLANE_COLUMNS 32
#define PLANE_ROWS 32
#define NUM_SPRITES 64
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

struct Core;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:3;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
        uint8_t size:2; // 1x1 - 4x4 characters
    };
    uint8_t value;
};

// 4 bytes
struct Sprite {
    uint8_t x;
    uint8_t y;
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Cell ================

// 2 bytes
struct Cell {
    uint8_t character;
    union CharacterAttributes attr;
};

// ================ Plane ================

// 2048 bytes
struct Plane {
    struct Cell cells[PLANE_ROWS][PLANE_COLUMNS];
};

// ===========================================
// ================ Video RAM ================
// ===========================================

// 8 KB
struct VideoRam {
    struct Character characters[NUM_CHARACTERS]; // 4 KB
    struct Plane planeA; // 2 KB
    struct Plane planeB; // 2 KB
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 256 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_PALETTES * 4]; // 32 bytes
};

union DisplayAttributes {
    struct {
        uint8_t spritesEnabled:1;
        uint8_t planeAEnabled:1;
        uint8_t planeBEnabled:1;
        uint8_t planeACellSize:1;
        uint8_t planeBCellSize:1;
    };
    uint8_t value;
};

union ScrollMSB {
    struct {
        uint8_t aX:1;
        uint8_t aY:1;
        uint8_t bX:1;
        uint8_t bY:1;
    };
    uint8_t value;
};

struct VideoRegisters {
    union DisplayAttributes attr;
    uint8_t scrollAX;
    uint8_t scrollAY;
    uint8_t scrollBX;
    uint8_t scrollBY;
    union ScrollMSB scrollMSB;
    uint8_t rasterLine;
};

// ===========================================
// ================ Functions ================
// ===========================================

void video_renderScreen(struct Core *core, uint32_t *outputRGB);

#endif /* video_chip_h */
