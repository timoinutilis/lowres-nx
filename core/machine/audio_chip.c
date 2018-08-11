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
    
    int i = 0;
    while (i < numSamples)
    {
        int16_t leftOutput = 0;
        int16_t rightOutput = 0;
        
        internals->accumulatorError += 0x10000;
        int errorDiffusionFactor = internals->accumulatorError / outputFrequency;
        internals->accumulatorError -= errorDiffusionFactor * outputFrequency;
        
        for (int i = 0; i < NUM_VOICES; i++)
        {
            struct Voice *voice = &registers->voices[i];
            struct VoiceInternals *voiceIn = &internals->voices[i];
            
            uint16_t freq = (voice->frequencyHigh << 8) | voice->frequencyLow;
            
            uint32_t accumulatorLast = voiceIn->accumulator;
            uint32_t accumulator = voiceIn->accumulator + (freq * errorDiffusionFactor);
            voiceIn->accumulator = accumulator;
            
            uint16_t accu16 = (accumulator >> 4) & 0xFFFF;
            uint16_t sample = 0x7FFF; // silence
            
            if (voice->wave == WaveTypeSawtooth)
            {
                sample = accu16;
            }
            else if (voice->wave == WaveTypePulse)
            {
                sample = ((accu16 >> 8) > voice->pulseWidth) ? 0xFFFF : 0x0000;
            }
            else if (voice->wave == WaveTypeTriangle)
            {
                sample = ((accu16 & 0x8000) ? ~(accu16 << 1) : (accu16 << 1));
            }
            else if (voice->wave == WaveTypeNoise)
            {
                if ((accumulator & 0x10000) != (accumulatorLast & 0x10000))
                {
                    uint16_t r = voiceIn->noiseRandom;
                    uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5) ) & 1;
                    voiceIn->noiseRandom = (r >> 1) | (bit << 15);
                }
                sample = voiceIn->noiseRandom & 0xFFFF;
            }
            
            int16_t voiceSample = (((int32_t)(sample - 0x7FFF)) * voice->volume) >> 6; // 4 bit for volume, 2 bit for global
            if (registers->mixer.value & (16 << i))
            {
                leftOutput += voiceSample;
            }
            if (registers->mixer.value & (1 << i))
            {
                rightOutput += voiceSample;
            }
        }
        
        stereoOutput[i++] = leftOutput;
        stereoOutput[i++] = rightOutput;
    }
}
