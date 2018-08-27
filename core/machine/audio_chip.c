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

const double envRates[16] = {
    256.0 / 0.002,
    256.0 / 0.008,
    256.0 / 0.016,
    256.0 / 0.024,
    256.0 / 0.038,
    256.0 / 0.056,
    256.0 / 0.068,
    256.0 / 0.080,
    256.0 / 0.100,
    256.0 / 0.250,
    256.0 / 0.500,
    256.0 / 0.800,
    256.0 / 1.0,
    256.0 / 3.0,
    256.0 / 5.0,
    256.0 / 8.0
};

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
            for (int v = 0; v < NUM_VOICES; v++)
            {
                struct Voice *voice = &registers->voices[v];
                struct VoiceInternals *voiceIn = &internals->voices[v];
                
                int freq = (voice->frequencyHigh << 8) | voice->frequencyLow;
                if (freq == 0) continue;
                
                int volume = voice->volume;
                int pulseWidth = voice->pulseWidth;
                
                // --- LFO ---
                
                double lfoFreq = 4;
                double lfoAccumulator = voiceIn->lfoAccumulator + lfoFreq * 65536.0 / (double)outputFrequency;
                if (lfoAccumulator >= overflow)
                {
                    // avoid overflow and loss of precision
                    lfoAccumulator -= overflow;
                }
                voiceIn->lfoAccumulator = lfoAccumulator;
                uint16_t lfoAccu8 = ((uint32_t)voiceIn->lfoAccumulator >> 8) & 0xFF;
                
                uint8_t lfoSample = ((lfoAccu8 & 0x80) ? ~(lfoAccu8 << 1) : (lfoAccu8 << 1));
                
                int freqAmount = 2;
                int volAmount = 2;
                int pwAmount = 2;
                
                freq += freq * lfoSample * freqAmount >> 11;
                if (freq < 1) freq = 1;
                if (freq > 65535) freq = 65535;
                
                volume += volume * lfoSample * volAmount >> 12;
                if (volume < 0) volume = 0;
                if (volume > 255) volume = 255;
                
                pulseWidth += lfoSample * pwAmount >> 4;
                if (pulseWidth < 2) pulseWidth = 2;
                if (pulseWidth > 252) pulseWidth = 252;
//                if (i == 0 && v == 0) printf("PW %d\n", pulseWidth);
                
                // --- WAVEFORM GENERATOR ---
                
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
                    sample = ((accu16 >> 8) > pulseWidth) ? 0xFFFF : 0x0000;
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
                
                // --- ENVELOPE GENERATOR ---
                
                if (!voice->attr.gate)
                {
                    voiceIn->envState = EnvStateRelease;
                }
                
                switch (voiceIn->envState) {
                    case EnvStateAttack:
                        voiceIn->envCounter += envRates[voice->envA] / outputFrequency;
                        if (voiceIn->envCounter >= 255.0)
                        {
                            voiceIn->envCounter = 255.0;
                            voiceIn->envState = EnvStateDecay;
                        }
                        break;
                        
                    case EnvStateDecay:
                        if (voiceIn->envCounter > voice->envS * 16.0)
                        {
                            voiceIn->envCounter -= envRates[voice->envD] / 3.0 / outputFrequency;
                        }
                        break;
                        
                    case EnvStateRelease:
                        if (voiceIn->envCounter > 0.0)
                        {
                            voiceIn->envCounter -= envRates[voice->envR] / 3.0 / outputFrequency;
                            if (voiceIn->envCounter < 0.0)
                            {
                                voiceIn->envCounter = 0.0;
                            }
                        }
                        break;
                }
                
                // --- OUTPUT ---
                
                volume = volume * (int)voiceIn->envCounter >> 8;
                int16_t voiceSample = (((int32_t)(sample - 0x7FFF)) * volume) >> 10; // 8 bit for volume, 2 bit for global
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
