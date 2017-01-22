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
#define NUM_COLORS 64
#define PLANE_COLUMNS 32
#define PLANE_ROWS 32
#define WINDOW_COLUMNS 16
#define WINDOW_ROWS 16
#define NUM_SPRITES 64
#define SPRITE_OFFSET_X 32
#define SPRITE_OFFSET_Y 32

// ================ Character ================

// 16 bytes
typedef struct {
    uint16_t data[8];
} Character;

// ================ Character Bank ================

// 4096 bytes
typedef struct {
    Character characters[NUM_CHARACTERS];
} CharacterBank;

// ================ Sprite ================

// 8 bytes
typedef struct {
    uint8_t x;
    uint8_t y;
    uint8_t character;
    union {
        struct {
            uint8_t attr_palette:4;
            uint8_t attr_bank:1;
            uint8_t attr_flipX:1;
            uint8_t attr_flipY:1;
            uint8_t attr_priority:1;
        };
        uint8_t attributes1;
    };
    union {
        struct {
            uint8_t attr_width:2; // 1-4 characters
            uint8_t attr_height:2; // 1-4 characters
        };
        uint8_t attributes2;
    };
    uint8_t reserved[3];
} Sprite;

// ================ Cell ================

// 2 bytes
typedef struct {
    uint8_t character;
    union {
        struct {
            uint8_t attr_palette:4;
            uint8_t attr_bank:1;
            uint8_t attr_flipX:1;
            uint8_t attr_flipY:1;
            uint8_t attr_priority:1;
        };
        uint8_t attributes;
    };
} Cell;

// ================ Plane ================

// 2048 bytes
typedef struct {
    Cell cells[PLANE_ROWS][PLANE_COLUMNS];
} Plane;

// ================ Window ================

// 512 bytes
typedef struct {
    Cell cells[WINDOW_ROWS][WINDOW_COLUMNS];
} Window;

// ===========================================
// ================ Video RAM ================
// ===========================================

// 16 KB
typedef struct {
    CharacterBank characterBanks[NUM_CHARACTER_BANKS]; // 8 KB
    Plane planeB; // 2 KB
    Plane planeA; // 2 KB
    Window window; // 512 bytes
    Sprite sprites[NUM_SPRITES]; // 512 bytes
    uint8_t reserved[3072]; // 3 KB
} VideoRam;

// =================================================
// ================ Video Registers ================
// =================================================

typedef struct {
    uint8_t colors[NUM_COLORS]; // 64 bytes
    union {
        struct {
            uint8_t attr_romCells:1;
            uint8_t attr_romSprites:1;
        };
        uint8_t attributes;
    };
    uint8_t scrollAX;
    uint8_t scrollAY;
    uint8_t scrollBX;
    uint8_t scrollBY;
} VideoRegisters;

// ===========================================
// ================ Functions ================
// ===========================================

void LRC_renderScreen(VideoRegisters *reg, VideoRam *ram, uint8_t *outputRGB);

#endif /* video_interface_h */
