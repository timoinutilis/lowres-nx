//
//  machine.h
//  LowRes Core iOS
//
//  Created by Timo Kloss on 22/1/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

#ifndef machine_h
#define machine_h

#include <stdio.h>
#include "io_interface.h"
#include "video_interface.h"
#include "audio_interface.h"

typedef struct {
    
    // 0x00000
    uint8_t cartridgeRom[0x80000]; // 512 KB
    
    // 0x80000
    VideoRam videoRam; // 16 KB
    
    // 0x84000
    IORegisters ioRegisters;
    uint8_t reserved1[0x100 - sizeof(IORegisters)];
    
    // 0x84100
    VideoRegisters videoRegisters;
    uint8_t reserved2[0x100 - sizeof(VideoRegisters)];
    
    // 0x84200
    AudioRegisters audioRegisters;
    uint8_t reserved3[0x100 - sizeof(AudioRegisters)];
    
    // 0x84300
    uint8_t reserved4[0x8D000 - 0x84300];
    
    // 0x8D000
    uint8_t cartridgeBackupRam[0x2000]; // 8 KB
    
    // 0x8F000
    CharacterBank characterRom; // 4 KB
    
    // 0x90000
    uint8_t workingRam[0x10000]; // 64 KB
    
} Machine;

void LRC_initMachine(Machine *machine);

#endif /* machine_h */
