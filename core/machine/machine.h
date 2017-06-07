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

#ifndef machine_h
#define machine_h

#include <stdio.h>
#include <stdbool.h>
#include "io_chip.h"
#include "video_chip.h"
#include "audio_chip.h"

// 64 KB
struct Machine {
    
    // 0x0000
    uint8_t cartridgeRom[0x8000]; // 32 KB
    
    // 0x8000
    struct VideoRam videoRam; // 8 KB

    // 0xA000
    uint8_t workingRam[0x3C00]; // 15 KB
    
    // 0xDC00
    uint8_t persistentRam[0x400]; // 1 KB

    // 0xE000
    struct CharacterBank characterRom; // 4 KB

    // 0xF000
    struct SpriteRegisters spriteRegisters; // 320 B
    uint8_t reserved4[0x200 - sizeof(struct SpriteRegisters)];
    
    // 0xF200
    struct ColorRegisters colorRegisters; // 32 B
    
    // 0xF220
    uint8_t reserved[0xFF40 - 0xF220];
    
    // 0xFF40
    struct VideoRegisters videoRegisters;
    uint8_t reserved3[0x40 - sizeof(struct VideoRegisters)];
    
    // 0xFF80
    struct AudioRegisters audioRegisters;
    uint8_t reserved2[0x40 - sizeof(struct AudioRegisters)];

    // 0xFFC0
    struct IORegisters ioRegisters;
    uint8_t reserved1[0x40 - sizeof(struct IORegisters)];
};

void machine_init(struct Machine *machine);
int machine_peek(struct Machine *machine, int address);
bool machine_poke(struct Machine *machine, int address, int value);

#endif /* machine_h */
