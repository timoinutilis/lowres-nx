//
//  machine.c
//  LowRes Core iOS
//
//  Created by Timo Kloss on 22/1/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

#include "machine.h"
#include <assert.h>
#include <string.h>
#include "character_rom.h"

void LRC_initMachine(Machine *machine)
{
    assert(sizeof(VideoRam) == 0x4000);
    assert(sizeof(CharacterBank) == 0x1000);
    assert(sizeof(Machine) == 0xA0000);
    assert(sizeof(IORegisters) < 0x100);
    assert(sizeof(VideoRegisters) < 0x100);
    assert(sizeof(IORegisters) < 0x100);
    
    // Copy character ROM data to machine
    memcpy((CharacterBank *)&machine->characterRom, CharacterRom, sizeof(CharacterBank));
}
