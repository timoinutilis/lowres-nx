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
    256.0 / 0.03,
    256.0 / 0.06,
    256.0 / 0.09,
    256.0 / 0.14,
    256.0 / 0.21,
    256.0 / 0.31,
    256.0 / 0.47,
    256.0 / 0.70,
    256.0 / 1.0,
    256.0 / 1.6,
    256.0 / 2.4,
    256.0 / 3.5,
    256.0 / 5.0,
    256.0 / 8.0,
    256.0 / 12.0
};

const double lfoRates[16] = {
    0.03 * 256.0,
    0.06 * 256.0,
    0.09 * 256.0,
    0.14 * 256.0,
    0.21 * 256.0,
    0.31 * 256.0,
    0.47 * 256.0,
    0.70 * 256.0,
    1.0 * 256.0,
    1.6 * 256.0,
    2.4 * 256.0,
    3.5 * 256.0,
    5.0 * 256.0,
    8.0 * 256.0,
    12.0 * 256.0,
    18.0 * 256.0
};

const int lfoAmounts[16] = {
    0,
    1,
    2,
    4,
    6,
    9,
    12,
    17,
    24,
    34,
    48,
    67,
    93,
    131,
    183,
    256
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
                
                int volume = voice->volume << 4;
                int pulseWidth = voice->pulseWidth << 4;
                
                // --- LFO ---
                
                if (!voiceIn->lfoHold)
                {
                    double lfoRate = lfoRates[voice->lfoFrequency];
                    double lfoAccumulator = voiceIn->lfoAccumulator + lfoRate / (double)outputFrequency;
                    if (voice->lfoAttr.envMode && lfoAccumulator >= 255.0)
                    {
                        lfoAccumulator = 255.0;
                        voiceIn->lfoHold = true;
                    }
                    else if (lfoAccumulator >= 256.0)
                    {
                        // avoid overflow and loss of precision
                        lfoAccumulator -= 256.0;
                    }
                    voiceIn->lfoAccumulator = lfoAccumulator;
                }
                uint8_t lfoAccu8 = voiceIn->lfoAccumulator;
                uint8_t lfoSample = 0;
                
                if (voice->lfoAttr.wave)
                {
                    // Sawtooth LFO
                    lfoSample = ~lfoAccu8;
                }
                else
                {
                    // Triangle LFO
                    lfoSample = ((lfoAccu8 & 0x80) ? ~(lfoAccu8 << 1) : (lfoAccu8 << 1));
                }
                
                int freqAmount = lfoAmounts[voice->lfoOscAmount];
                int volAmount = voice->lfoVolAmount;
                int pwAmount = voice->lfoPWAmount;
                
                int freqMod = freq * lfoSample * freqAmount >> 16;
                if (voice->lfoAttr.invert) freq -= freqMod; else freq += freqMod;
                if (freq < 1) freq = 1;
                if (freq > 65535) freq = 65535;
                
                if (voice->lfoAttr.invert)
                {
                    volume -= volume * lfoSample * volAmount >> 12;
                }
                else
                {
                    volume -= volume * (~lfoSample & 0xFF) * volAmount >> 12;
                }
                if (volume < 0) volume = 0;
                if (volume > 255) volume = 255;
                
                int pwMod = lfoSample * pwAmount >> 4;
                if (voice->lfoAttr.invert) pulseWidth -= pwMod; else pulseWidth += pwMod;
                if (pulseWidth < 0) pulseWidth = 0;
                if (pulseWidth > 254) pulseWidth = 254;
                
//                if (i == 0 && v == 0) printf("pulseWidth %d\n", pulseWidth);
                
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
                
                // --- TIMEOUT ---
                
                if (voice->attr.timeout)
                {
                    voiceIn->timeoutCounter -= 60.0 / outputFrequency;
                    if (voiceIn->timeoutCounter <= 0.0)
                    {
                        voiceIn->timeoutCounter = 0.0;
                        voice->attr.gate = 0;
                    }
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
                            voiceIn->envCounter -= envRates[voice->envD] / outputFrequency;
                        }
                        break;
                        
                    case EnvStateRelease:
                        if (voiceIn->envCounter > 0.0)
                        {
                            voiceIn->envCounter -= envRates[voice->envR] / outputFrequency;
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

void audio_onVoiceAttrChange(struct Core *core, int index)
{
    struct Voice *voice = &core->machine->audioRegisters.voices[index];
    if (voice->attr.gate)
    {
        struct VoiceInternals *voiceIn = &core->machineInternals->audioInternals.voices[index];
        voiceIn->envState = EnvStateAttack;
        voiceIn->lfoHold = false;
        voiceIn->timeoutCounter = voice->length;
        if (voice->lfoAttr.envMode || voice->lfoAttr.trigger)
        {
            voiceIn->lfoAccumulator = 0.0;
        }
    }
}
