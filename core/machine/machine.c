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

#include "machine.h"
#include <assert.h>
#include <string.h>
#include "character_rom.h"

void LRC_initMachine(struct Machine *machine)
{
    assert(sizeof(struct VideoRam) == 0x4000);
    assert(sizeof(struct CharacterBank) == 0x1000);
    assert(sizeof(struct Machine) == 0xA0000);
    assert(sizeof(struct IORegisters) < 0x100);
    assert(sizeof(struct VideoRegisters) < 0x100);
    assert(sizeof(struct IORegisters) < 0x100);
    
    // Copy character ROM data to machine
    memcpy((struct CharacterBank *)&machine->characterRom, CharacterRom, sizeof(struct CharacterBank));
}
