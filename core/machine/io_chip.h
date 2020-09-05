//
// Copyright 2016 Timo Kloss
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
