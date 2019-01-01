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

#ifndef io_chip_h
#define io_chip_h

#include <stdint.h>

#define NUM_GAMEPADS 2

// ================ Gamepad ================

union Gamepad {
    struct {
        uint8_t up:1;
        uint8_t down:1;
        uint8_t left:1;
        uint8_t right:1;
        uint8_t buttonA:1;
        uint8_t buttonB:1;
    };
    uint8_t value;
};

// ================ Status ================

union IOStatus {
    struct {
        uint8_t pause:1;
        uint8_t touch:1;
    };
    uint8_t value;
};

// ================ Attributes ================

union IOAttributes {
    struct {
        uint8_t gamepadsEnabled:2; // 0: off, 1...2: number of players
        uint8_t keyboardEnabled:1;
        uint8_t touchEnabled:1;
    };
    uint8_t value;
};

// ===============================================
// ================ I/O Registers ================
// ===============================================

struct IORegisters {
    union Gamepad gamepads[NUM_GAMEPADS]; // 2 bytes
    uint8_t touchX;
    uint8_t touchY;
    char key;
    union IOStatus status;
    union IOAttributes attr;
};

#endif /* io_chip_h */
