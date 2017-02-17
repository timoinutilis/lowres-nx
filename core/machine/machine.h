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

#ifndef machine_h
#define machine_h

#include <stdio.h>
#include "io_interface.h"
#include "video_interface.h"
#include "audio_interface.h"

// 64 KB
struct Machine {
    
    // 0x0000
    uint8_t cartridgeRom[0x8000]; // 32 KB
    
    // 0x8000
    struct VideoRam videoRam; // 8 KB

    // 0xA000
    uint8_t workingRam[0x2000]; // 8 KB
    
    // 0xC000
    uint8_t persistentRam[0x2000]; // 8 KB

    // 0xE000
    struct CharacterBank characterRom; // 4 KB

    // 0xF000
    struct SpriteRegisters spriteRegisters; // 512 B
    
    // 0xF200
    struct ColorRegisters colorRegisters; // 64 B
    
    // 0xF240
    uint8_t reserved2[0xFF40 - 0xF240];
    
    // 0xFF40
    struct VideoRegisters videoRegisters;
    uint8_t reserved3[0x40 - sizeof(struct VideoRegisters)];
    
    // 0xFF80
    struct AudioRegisters audioRegisters;
    uint8_t reserved4[0x40 - sizeof(struct AudioRegisters)];

    // 0xFFC0
    struct IORegisters ioRegisters;
    uint8_t reserved5[0x40 - sizeof(struct IORegisters)];
};

void LRC_initMachine(struct Machine *machine);

#endif /* machine_h */
