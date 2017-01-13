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

#ifndef core_h
#define core_h

#include <stdio.h>
#include "io_interface.h"
#include "video_interface.h"
#include "audio_interface.h"

// 64 KB
typedef struct {
    // ==== RAM (48 KB) ====
    
    // 0x0000
    VideoRam videoRam;
    
    // 0x4000
    uint8_t workingRam[0x4000];
    
    // 0x8000
    uint8_t systemRam[0x4000];
    
    // ==== ROM (8 KB) ====
    
    // 0xC000
    CharacterBank characterRom;
    
    // 0xD000
    uint8_t reservedRom[0x1000];
    
    // ==== Registers (8 KB) ====
    
    // 0xE000
    VideoRegisters videoRegisters;
    
    // 0xE400
    IORegisters ioRegisters;
    
    // 0xE500
    uint8_t reservedRegisters[0x1B00];
} LRCore;

void LRC_init(LRCore *core);
void LRC_update(LRCore *core);

#endif /* core_h */
