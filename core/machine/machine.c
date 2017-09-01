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

#include "machine.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>

void machine_init(struct Machine *machine)
{
    assert(sizeof(struct Machine) == 0x10000);
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
        // cartridge ROM or outside RAM
        return false;
    }
    if (address >= 0xE100 && address < 0xFE00)
    {
        // reserved memory
        return false;
    }
    if (address >= 0xFF80)
    {
        // reserved registers
        return false;
    }
    *(uint8_t *)((uint8_t *)machine + address) = value & 0xFF;
    return true;
}
