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

#ifndef machine_h
#define machine_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "io_chip.h"
#include "video_chip.h"
#include "audio_chip.h"

#define PERSISTENT_RAM_SIZE 4096

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
    uint8_t persistentRam[PERSISTENT_RAM_SIZE]; // 4 KB
    
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
    
    // 0xFF70
    struct IORegisters ioRegisters;
    uint8_t reservedIO[0x10 - sizeof(struct IORegisters)];
    
    // 0xFF80
    uint8_t reservedRegisters[0x10000 - 0xFF80];
};

struct MachineInternals {
    struct AudioInternals audioInternals;
    bool hasAccessedPersistent;
    bool hasChangedPersistent;
    bool isEnergySaving;
    int energySavingTimer;
};

void machine_init(struct Core *core);
void machine_reset(struct Core *core);
int machine_peek(struct Core *core, int address);
bool machine_poke(struct Core *core, int address, int value);
void machine_enableAudio(struct Core *core);
void machine_suspendEnergySaving(struct Core *core, int numUpdates);

#endif /* machine_h */
