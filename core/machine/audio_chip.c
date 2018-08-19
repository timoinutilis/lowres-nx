//
// Copyright 2016-2018 Timo Kloss
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

#include "audio_chip.h"
#include "core.h"
#include <math.h>

void audio_reset(struct Core *core)
{
    struct AudioInternals *internals = &core->machineInternals->audioInternals;
    
    for (int i = 0; i < NUM_VOICES; i++)
    {
        struct VoiceInternals *voiceIn = &internals->voices[i];
        voiceIn->noiseRandom = 1;
    }
}

void audio_renderAudio(struct Core *core, int16_t *stereoOutput, int numSamples, int outputFrequency)
{
    struct AudioRegisters *registers = &core->machine->audioRegisters;
    struct AudioInternals *internals = &core->machineInternals->audioInternals;
    
    double overflow = 0xFFFFFF;
    
    int i = 0;
    while (i < numSamples)
    {
        int16_t leftOutput = 0;
        int16_t rightOutput = 0;
        
        if (registers->attr.audioEnabled)
        {
            for (int i = 0; i < NUM_VOICES; i++)
            {
                struct Voice *voice = &registers->voices[i];
                struct VoiceInternals *voiceIn = &internals->voices[i];
                
                uint16_t freq = (voice->frequencyHigh << 8) | voice->frequencyLow;
                if (freq == 0) continue;
                
                uint16_t accu16Last = ((uint32_t)voiceIn->accumulator >> 4) & 0xFFFF;
                double accumulator = voiceIn->accumulator + (double)freq * 65536.0 / (double)outputFrequency;
                if (accumulator >= overflow)
                {
                    // avoid overflow and loss of precision
                    accumulator -= overflow;
                }
                voiceIn->accumulator = accumulator;
                uint16_t accu16 = ((uint32_t)voiceIn->accumulator >> 4) & 0xFFFF;
                
                uint16_t sample = 0x7FFF; // silence
                
                enum WaveType waveType = voice->attr.wave;
                if (waveType == WaveTypeSawtooth)
                {
                    sample = accu16;
                }
                else if (waveType == WaveTypePulse)
                {
                    sample = ((accu16 >> 8) > voice->pulseWidth) ? 0xFFFF : 0x0000;
                }
                else if (waveType == WaveTypeTriangle)
                {
                    sample = ((accu16 & 0x8000) ? ~(accu16 << 1) : (accu16 << 1));
                }
                else if (waveType == WaveTypeNoise)
                {
                    if ((accu16 & 0x1000) != (accu16Last & 0x1000))
                    {
                        uint16_t r = voiceIn->noiseRandom;
                        uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5) ) & 1;
                        voiceIn->noiseRandom = (r >> 1) | (bit << 15);
                    }
                    sample = voiceIn->noiseRandom & 0xFFFF;
                }
                
                int16_t voiceSample = (((int32_t)(sample - 0x7FFF)) * voice->volume) >> 10; // 8 bit for volume, 2 bit for global
                if (voice->attr.mixL)
                {
                    leftOutput += voiceSample;
                }
                if (voice->attr.mixR)
                {
                    rightOutput += voiceSample;
                }
            }
        }
        
        stereoOutput[i++] = leftOutput;
        stereoOutput[i++] = rightOutput;
    }
}
