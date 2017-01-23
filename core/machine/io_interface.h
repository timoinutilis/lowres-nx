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

#ifndef io_interface_h
#define io_interface_h

#define NUM_GAMEPADS 2

// ================ Gamepad ================

union Gamepad {
    struct {
        uint8_t status_up:1;
        uint8_t status_down:1;
        uint8_t status_left:1;
        uint8_t status_right:1;
        uint8_t status_buttonA:1;
        uint8_t status_buttonB:1;
    };
    uint8_t status;
} ;

// ===============================================
// ================ I/O Registers ================
// ===============================================

struct IORegisters {
    union Gamepad gamepads[NUM_GAMEPADS]; // 2 bytes
    uint8_t mouseX;
    uint8_t mouseY;
    uint8_t key;
    union {
        struct {
            uint8_t status_pause:1;
            uint8_t status_mouseButton:1;
        };
        uint8_t status;
    };
};

#endif /* io_interface_h */
