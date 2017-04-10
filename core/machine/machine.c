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
#include <stdbool.h>
#include "character_rom.h"

void machine_init(struct Machine *machine)
{
    assert(sizeof(struct Machine) == 0x10000);
    
    // Copy character ROM data to machine
    memcpy((struct CharacterBank *)&machine->characterRom, CharacterRom, sizeof(struct CharacterBank));
}

int machine_peek(struct Machine *machine, int address)
{
    if (address < 0 || address > 0xFFFF)
    {
        return -1;
    }
    return *(uint8_t *)((uint8_t *)machine + address);
}

bool machine_poke(struct Machine *machine, int address, int value)
{
    if (address < 0x8000 || address > 0xFFFF)
    {
        return false;
    }
    *(uint8_t *)((uint8_t *)machine + address) = value & 0xFF;
    return true;
}
