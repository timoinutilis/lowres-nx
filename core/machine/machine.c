//
// Copyright 2016-2018 Timo Kloss
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

#include "machine.h"
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "core.h"

void machine_init(struct Core *core)
{
    assert(sizeof(struct Machine) == 0x10000);
}

void machine_reset(struct Core *core)
{
    memset(core->machine, 0, sizeof(struct Machine));
    memset(core->machineInternals, 0, sizeof(struct MachineInternals));
    audio_reset(core);
}

int machine_peek(struct Core *core, int address)
{
    if (address < 0 || address > 0xFFFF)
    {
        return -1;
    }
    if (address >= 0xE000 && address < 0xF000) // persistent
    {
        if (!core->machineInternals->hasAccessedPersistent)
        {
            delegate_persistentRamWillAccess(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
            core->machineInternals->hasAccessedPersistent = true;
        }
    }
    
    // read byte
    return *(uint8_t *)((uint8_t *)core->machine + address);
}

bool machine_poke(struct Core *core, int address, int value)
{
    if (address < 0x8000 || address > 0xFFFF)
    {
        // cartridge ROM or outside RAM
        return false;
    }
    if (address >= 0xF000 && address < 0xFE00)
    {
        // reserved memory
        return false;
    }
    if (address >= 0xFF80)
    {
        // reserved registers
        return false;
    }
    if (address == 0xFF76) // IO attributes
    {
        // check for illegal input change (gamepad <-> touch)
        union IOAttributes currAttr = core->machine->ioRegisters.attr;
        union IOAttributes newAttr;
        newAttr.value = value;
        if (currAttr.gamepadsEnabled > 0 && (newAttr.gamepadsEnabled == 0 || newAttr.touchEnabled))
        {
            return false;
        }
        if (currAttr.touchEnabled && (newAttr.touchEnabled == 0 || newAttr.gamepadsEnabled > 0))
        {
            return false;
        }
    }
    else if (address >= 0xE000 && address < 0xF000) // persistent
    {
        if (!core->machineInternals->hasAccessedPersistent)
        {
            delegate_persistentRamWillAccess(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
            core->machineInternals->hasAccessedPersistent = true;
        }
        core->machineInternals->hasChangedPersistent = true;
    }
    
    // write byte
    *(uint8_t *)((uint8_t *)core->machine + address) = value & 0xFF;
    
    if (address == 0xFF76) // IO attributes
    {
        delegate_controlsDidChange(core);
    }
    else if (address >= 0xFF40 && address < 0xFF70) // audio
    {
        machine_enableAudio(core);
    }
    return true;
}

void machine_enableAudio(struct Core *core)
{
    if (!core->machineInternals->audioInternals.audioEnabled)
    {
        core->machineInternals->audioInternals.audioEnabled = true;
        delegate_controlsDidChange(core);
    }
}

void machine_suspendEnergySaving(struct Core *core, int numUpdates)
{
    if (core->machineInternals->energySavingTimer < numUpdates)
    {
        core->machineInternals->energySavingTimer = numUpdates;
    }
}
