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

#define SOUND_SIZE 8
#define PATTERN_SIZE 4
#define ROW_SIZE 3

struct TrackRow {
    int note;
    int sound;
    int volume;
    int command;
    int parameter;
};

void audlib_updateMusic(struct AudioLib *lib);
void audlib_setPitch(struct Voice *voice, float pitch);
bool audlib_isPatternEmpty(struct AudioLib *lib, int pattern);
int audlib_getLoopStart(struct AudioLib *lib, int pattern);
int audlib_getLoop(struct AudioLib *lib, int pattern, int param);
int audlib_getTrack(struct AudioLib *lib, int pattern, int voice);
struct TrackRow audlib_getTrackRow(struct AudioLib *lib, int track, int row);
void audlib_playRow(struct AudioLib *lib, int track, int row, int voice);
void audlib_command(struct AudioLib *lib, struct Voice *voice, int command, int parameter);


void audlib_play(struct AudioLib *lib, int voiceIndex, float pitch, int len, int sound)
{
    struct Core *core = lib->core;
    struct Voice *voice = &core->machine->audioRegisters.voices[voiceIndex];
    
    audlib_setPitch(voice, pitch);
    
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
    
    machine_enableAudio(core);
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
    lib->pattern = startPattern;
    lib->speed = 8;
    lib->tick = -1;
    lib->row = 0;
    lib->state = MusicStatePlaying;
    
    machine_enableAudio(lib->core);
}

void audlib_playTrack(struct AudioLib *lib, int voiceIndex, int track)
{
    
}

void audlib_stopAll(struct AudioLib *lib)
{
    lib->state = MusicStateOff;
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

void audlib_update(struct AudioLib *lib)
{
    if (lib->state == MusicStatePlaying)
    {
        audlib_updateMusic(lib);
    }
}


void audlib_updateMusic(struct AudioLib *lib)
{
    if (lib->tick == -1)
    {
        lib->tick = 0;
    }
    else if (lib->tick == 0)
    {
        lib->row = (lib->row + 1) % NUM_ROWS;
        if (lib->row == 0 && lib->state == MusicStatePlaying)
        {
            if (audlib_getLoop(lib, lib->pattern, 2) == 1)
            {
                audlib_stopAll(lib);
                return;
            }
            if (audlib_getLoop(lib, lib->pattern, 1) == 1)
            {
                lib->pattern = audlib_getLoopStart(lib, lib->pattern);
            }
            else
            {
                int p = lib->pattern + 1;
                if (p < NUM_PATTERNS)
                {
                    if (audlib_isPatternEmpty(lib, p))
                    {
                        audlib_stopAll(lib);
                        return;
                    }
                    else
                    {
                        lib->pattern = p;
                    }
                }
                else
                {
                    audlib_stopAll(lib);
                    return;
                }
            }
        }
    }
    if (lib->tick == 0)
    {
        for (int voice = 0; voice < NUM_VOICES; voice++)
        {
            int track = audlib_getTrack(lib, lib->pattern, voice);
            if (track >= 0)
            {
                audlib_playRow(lib, track, lib->row, voice);
            }
        }
    }
    lib->tick = (lib->tick + 1) % lib->speed;
}

void audlib_setPitch(struct Voice *voice, float pitch)
{
    int f = 16.0 * 440.0 * pow(2.0, (pitch - 58.0) / 12.0);
    voice->frequencyLow = f & 0xFF;
    voice->frequencyHigh = f >> 8;
}

int audlib_getSoundsAddress(struct AudioLib *lib)
{
    return lib->soundSourceAddress;
}

int audlib_getPatternsAddress(struct AudioLib *lib)
{
    return audlib_getSoundsAddress(lib) + NUM_SOUNDS * SOUND_SIZE;
}

int audlib_getTracksAddress(struct AudioLib *lib)
{
    return audlib_getPatternsAddress(lib) + NUM_PATTERNS * PATTERN_SIZE;
}

bool audlib_isPatternEmpty(struct AudioLib *lib, int pattern)
{
    for (int v = 0; v < NUM_VOICES; v++)
    {
        if (audlib_getTrack(lib, pattern, v) >= 0)
        {
            return false;
        }
    }
    return true;
}

int audlib_getLoopStart(struct AudioLib *lib, int pattern)
{
    for (int p = pattern; p >= 0; p--)
    {
        if (audlib_getLoop(lib, p, 0) == 1)
        {
            return p;
        }
    }
    return 0;
}

int audlib_getLoop(struct AudioLib *lib, int pattern, int param)
{
    int a = audlib_getPatternsAddress(lib) + pattern * PATTERN_SIZE + param;
    return machine_peek(lib->core, a) >> 7;
}

int audlib_getTrack(struct AudioLib *lib, int pattern, int voice)
{
    int a = audlib_getPatternsAddress(lib) + pattern * PATTERN_SIZE + voice;
    int track = machine_peek(lib->core, a) & 0x7F;
    if (track == 0x40)
    {
        track = -1;
    }
    return track;
}

struct TrackRow audlib_getTrackRow(struct AudioLib *lib, int track, int row)
{
    struct TrackRow trackRow;
    int a = audlib_getTracksAddress(lib) + track * NUM_ROWS * ROW_SIZE + row * ROW_SIZE;
    struct Core *core = lib->core;
    trackRow.note = machine_peek(core, a);
    int peek1 = machine_peek(core, a + 1);
    trackRow.sound = peek1 >> 4;
    trackRow.volume = peek1 & 0x0F;
    int peek2 = machine_peek(core, a + 2);
    trackRow.command = peek2 >> 4;
    trackRow.parameter = peek2 & 0x0F;
    return trackRow;
}

void audlib_playRow(struct AudioLib *lib, int track, int row, int voiceIndex)
{
    struct Core *core = lib->core;
    struct Voice *voice = &core->machine->audioRegisters.voices[voiceIndex];
    
    struct TrackRow trackRow = audlib_getTrackRow(lib, track, row);
    if (trackRow.note > 0 && trackRow.note < 255)
    {
        audlib_copySound(lib, trackRow.sound, voiceIndex);
    }
    if (trackRow.volume > 0)
    {
        voice->status.volume = trackRow.volume;
    }
    audlib_command(lib, voice, trackRow.command, trackRow.parameter);
    if (trackRow.note == 255)
    {
        voice->status.gate = 0;
    }
    else if (trackRow.note > 0)
    {
        audlib_setPitch(voice, trackRow.note);
        voice->status.init = 1;
        voice->status.gate = 1;
    }
}

void audlib_command(struct AudioLib *lib, struct Voice *voice, int command, int parameter)
{
    if (command == 0 && parameter == 0) return;
    switch (command)
    {
        case 0x00:
            voice->status.mix = parameter & 0x03;
            break;
        case 0x01:
            voice->envA = parameter;
            break;
        case 0x02:
            voice->envD = parameter;
            break;
        case 0x03:
            voice->envS = parameter;
            break;
        case 0x04:
            voice->envR = parameter;
            break;
        case 0x05:
            voice->lfoFrequency = parameter;
            break;
        case 0x06:
            voice->lfoOscAmount = parameter;
            break;
        case 0x07:
            voice->lfoVolAmount = parameter;
            break;
        case 0x08:
            voice->lfoPWAmount = parameter;
            break;
        case 0x09:
            voice->attr.pulseWidth = parameter;
            break;
        case 0x0A:
            if (parameter > 0)
            {
                lib->speed = parameter;
            }
            break;
        case 0x0F:
            switch (parameter)
            {
                case 0:
                    //TODO: break pattern
                    break;
                case 1:
                    voice->status.gate = 0;
                    voice->status.volume = 0;
                    break;
            }
            break;
    }
}
