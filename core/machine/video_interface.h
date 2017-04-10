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

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128
#define NUM_CHARACTERS 256
#define NUM_COLORS 64
#define PLANE_COLUMNS 32
#define PLANE_ROWS 32
#define NUM_SPRITES 64
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

struct LowResCore;

// ================ Character ================

// 16 bytes
struct Character {
    uint8_t data[16];
};

// ================ Character Bank ================

// 4096 bytes
struct CharacterBank {
    struct Character characters[NUM_CHARACTERS];
};

// ================ Sprite ================

union CharacterAttributes {
    struct {
        uint8_t palette:4;
        uint8_t bank:1;
        uint8_t flipX:1;
        uint8_t flipY:1;
        uint8_t priority:1;
    };
    uint8_t value;
};

// ================ Sprite ================

// 8 bytes
struct Sprite {
    uint8_t x;
    uint8_t y;
    uint8_t character;
    union CharacterAttributes attr1;
    union {
        struct {
            uint8_t width:2; // 1-4 characters
            uint8_t height:2; // 1-4 characters
        };
        uint8_t value;
    } attr2;
    uint8_t reserved[3];
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
    struct CharacterBank characterBank; // 4 KB
    struct Plane planeA; // 2 KB
    struct Plane planeB; // 2 KB
};

// =================================================
// ================ Video Registers ================
// =================================================

struct SpriteRegisters {
    struct Sprite sprites[NUM_SPRITES]; // 512 bytes
};

struct ColorRegisters {
    uint8_t colors[NUM_COLORS]; // 64 bytes
};

struct VideoRegisters {
    union {
        uint8_t attributes;
    };
    uint8_t scrollAX;
    uint8_t scrollAY;
    uint8_t scrollBX;
    uint8_t scrollBY;
    uint8_t rasterLine;
};

// ===========================================
// ================ Functions ================
// ===========================================

void LRC_renderScreen(struct LowResCore *core, uint8_t *outputRGB, int bytesPerLine);

#endif /* video_interface_h */
