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

struct Machine {
    
    // 0x00000
    uint8_t cartridgeRom[0x80000]; // 512 KB
    
    // 0x80000
    struct VideoRam videoRam; // 8 KB
    
    // 0x82000
    struct IORegisters ioRegisters;
    uint8_t reserved1[0x100 - sizeof(struct IORegisters)];
    
    // 0x82100
    struct VideoRegisters videoRegisters;
    uint8_t reserved2[0x200 - sizeof(struct VideoRegisters)];
    
    // 0x82300
    struct AudioRegisters audioRegisters;
    uint8_t reserved3[0x100 - sizeof(struct AudioRegisters)];
    
    // 0x82400
    uint8_t reserved4[0x8D000 - 0x82400];
    
    // 0x8D000
    uint8_t cartridgeBackupRam[0x2000]; // 8 KB
    
    // 0x8F000
    struct CharacterBank characterRom; // 4 KB
    
    // 0x90000
    uint8_t workingRam[0x10000]; // 64 KB
    
};

void LRC_initMachine(struct Machine *machine);

#endif /* machine_h */
