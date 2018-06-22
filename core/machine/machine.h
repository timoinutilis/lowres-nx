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
#include <stdint.h>
#include "io_chip.h"
#include "video_chip.h"
#include "audio_chip.h"

struct Core;

// 64 KB
struct Machine {
    
    // 0x0000
    uint8_t cartridgeRom[0x8000]; // 32 KB
    
    // 0x8000
    struct VideoRam videoRam; // 8 KB

    // 0xA000
    uint8_t workingRam[0x4000]; // 16 KB
    
    // 0xE000
    uint8_t persistentRam[0x100]; // 256 B
    uint8_t reserved1[0xF00];
    
    // 0xF000
    uint8_t reservedMemory[0xFE00 - 0xF000];

    // 0xFE00
    struct SpriteRegisters spriteRegisters; // 256 B
    
    // 0xFF00
    struct ColorRegisters colorRegisters; // 32 B
    
    // 0xFF20
    struct VideoRegisters videoRegisters;
    uint8_t reservedVideo[0x20 - sizeof(struct VideoRegisters)];
    
    // 0xFF40
    struct AudioRegisters audioRegisters;
    uint8_t reservedAudio[0x20 - sizeof(struct AudioRegisters)];

    // 0xFF60
    struct IORegisters ioRegisters;
    uint8_t reservedIO[0x20 - sizeof(struct IORegisters)];
    
    // 0xFF80
    uint8_t reservedRegisters[0x10000 - 0xFF80];
};

void machine_init(struct Core *core);
void machine_reset(struct Core *core);
int machine_peek(struct Core *core, int address);
bool machine_poke(struct Core *core, int address, int value);

#endif /* machine_h */
