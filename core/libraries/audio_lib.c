//
// Copyright 2018 Timo Kloss
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

#include "audio_lib.h"
#include "core.h"
#include <math.h>

void audlib_play(struct AudioLib *lib, int voiceIndex, float pitch, int len, int sound)
{
    struct Core *core = lib->core;
    struct Voice *voice = &core->machine->audioRegisters.voices[voiceIndex];
    
    int f = 16.0 * 440.0 * pow(2.0, (pitch - 58.0) / 12.0);
    voice->frequencyLow = f & 0xFF;
    voice->frequencyHigh = f >> 8;
    
    if (sound != -1)
    {
        audlib_copySound(lib, sound, voiceIndex);
    }
    
    if (len != -1)
    {
        voice->length = len;
        voice->attr.timeout = (len > 0) ? 1 : 0;
    }
    voice->status.init = 1;
    voice->status.gate = 1;
    
    if (!core->machineInternals->audioInternals.audioEnabled)
    {
        core->machineInternals->audioInternals.audioEnabled = true;
        delegate_controlsDidChange(core);
    }
}

void audlib_copySound(struct AudioLib *lib, int sound, int voiceIndex)
{
    int addr = lib->soundSourceAddress + sound * 8;
    int dest = 0xFF40 + voiceIndex * sizeof(struct Voice) + 4;
    for (int i = 0; i < 8; i++)
    {
        int peek = machine_peek(lib->core, addr++);
        machine_poke(lib->core, dest++, peek);
    }
    lib->core->interpreter->cycles += 8;
}

void audlib_playMusic(struct AudioLib *lib, int startPattern)
{
    
}

void audlib_playTrack(struct AudioLib *lib, int voiceIndex, int track)
{
    
}

void audlib_stopAll(struct AudioLib *lib)
{
    for (int i = 0; i < NUM_VOICES; i++)
    {
        struct Voice *voice = &lib->core->machine->audioRegisters.voices[i];
        voice->status.gate = 0;
    }
}

void audlib_stopVoice(struct AudioLib *lib, int voiceIndex)
{
    struct Voice *voice = &lib->core->machine->audioRegisters.voices[voiceIndex];
    voice->status.gate = 0;
}
