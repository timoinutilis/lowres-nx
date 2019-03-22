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
#include <string.h>

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
    0.12 * 256.0,
    0.16 * 256.0,
    0.23 * 256.0,
    0.32 * 256.0,
    0.44 * 256.0,
    0.62 * 256.0,
    0.87 * 256.0,
    1.2 * 256.0,
    1.7 * 256.0,
    2.4 * 256.0,
    3.3 * 256.0,
    4.7 * 256.0,
    6.6 * 256.0,
    9.2 * 256.0,
    12.9 * 256.0,
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

void audio_renderAudioBuffer(struct AudioRegisters *lifeRegisters, struct AudioRegisters *registers, struct AudioInternals *internals, int16_t *stereoOutput, int numSamples, int outputFrequency);


void audio_reset(struct Core *core)
{
    struct AudioInternals *internals = &core->machineInternals->audioInternals;
    
    for (int i = 0; i < NUM_VOICES; i++)
    {
        struct VoiceInternals *voiceIn = &internals->voices[i];
        voiceIn->noiseRandom = 0xABCD;
        voiceIn->lfoRandom = 0xABCD;
    }
    internals->writeBufferIndex = -1;
}

void audio_bufferRegisters(struct Core *core)
{
    struct AudioRegisters *registers = &core->machine->audioRegisters;
    struct AudioInternals *internals = &core->machineInternals->audioInternals;
    
    // next buffer
    int writeBufferIndex = internals->writeBufferIndex;
    if (writeBufferIndex >= 0)
    {
        writeBufferIndex = (writeBufferIndex + 1) % NUM_AUDIO_BUFFERS;
    }
    else
    {
        writeBufferIndex = NUM_AUDIO_BUFFERS / 2;
    }
    
    // copy registers to buffer
    memcpy(&internals->buffers[writeBufferIndex], registers, sizeof(struct AudioRegisters));
    
    // reset "init" flags
    for (int v = 0; v < NUM_VOICES; v++)
    {
        struct Voice *voice = &registers->voices[v];
        voice->status.init = 0;
    }
    
    internals->writeBufferIndex = writeBufferIndex;
}

void audio_renderAudio(struct Core *core, int16_t *stereoOutput, int numSamples, int outputFrequency)
{
    struct AudioInternals *internals = &core->machineInternals->audioInternals;
    struct AudioRegisters *lifeRegisters = &core->machine->audioRegisters;
    
    int numSamplesPerUpdate = outputFrequency / 60 * NUM_CHANNELS;
    int offset = 0;
    
    while (offset < numSamples)
    {
        if (offset + numSamplesPerUpdate > numSamples)
        {
            numSamplesPerUpdate = numSamples - offset;
        }
        int readBufferIndex = internals->readBufferIndex;
        audio_renderAudioBuffer(lifeRegisters, &internals->buffers[readBufferIndex], internals, &stereoOutput[offset], numSamplesPerUpdate, outputFrequency);
        if (internals->writeBufferIndex != -1 && internals->writeBufferIndex != readBufferIndex)
        {
            internals->readBufferIndex = (readBufferIndex + 1) % NUM_AUDIO_BUFFERS;
        }
        
        offset += numSamplesPerUpdate;
    }
}

void audio_renderAudioBuffer(struct AudioRegisters *lifeRegisters, struct AudioRegisters *registers, struct AudioInternals *internals, int16_t *stereoOutput, int numSamples, int outputFrequency)
{
    double overflow = 0xFFFFFF;
    
    for (int v = 0; v < NUM_VOICES; v++)
    {
        struct Voice *voice = &registers->voices[v];
        if (voice->status.init)
        {
            voice->status.init = 0;
            
            struct VoiceInternals *voiceIn = &internals->voices[v];
            voiceIn->envState = EnvStateAttack;
            voiceIn->lfoHold = false;
            voiceIn->timeoutCounter = voice->length;
            if (voice->lfoAttr.envMode || voice->lfoAttr.trigger)
            {
                voiceIn->lfoAccumulator = 0.0;
            }
        }
    }
    
    int i = 0;
    while (i < numSamples)
    {
        int16_t leftOutput = 0;
        int16_t rightOutput = 0;
        
        if (internals->audioEnabled)
        {
            for (int v = 0; v < NUM_VOICES; v++)
            {
                struct Voice *voice = &registers->voices[v];
                struct VoiceInternals *voiceIn = &internals->voices[v];
                
                int freq = (voice->frequencyHigh << 8) | voice->frequencyLow;
                if (freq == 0) continue;
                
                int volume = voice->status.volume << 4;
                int pulseWidth = voice->attr.pulseWidth << 4;
                
                // --- LFO ---
                
                uint8_t lfoAccu8Last = voiceIn->lfoAccumulator;
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
                
                enum LFOWaveType lfoWaveType = voice->lfoAttr.wave;
                switch (lfoWaveType)
                {
                    case LFOWaveTypeTriangle:
                    {
                        lfoSample = ((lfoAccu8 & 0x80) ? ~(lfoAccu8 << 1) : (lfoAccu8 << 1));
                        break;
                    }
                    case LFOWaveTypeSawtooth:
                    {
                        lfoSample = ~lfoAccu8;
                        break;
                    }
                    case LFOWaveTypeSquare:
                    {
                        lfoSample = (lfoAccu8 & 0x80) ? 0x00 : 0xFF;
                        break;
                    }
                    case LFOWaveTypeRandom:
                    {
                        if ((lfoAccu8 & 0x80) != (lfoAccu8Last & 0x80))
                        {
                            uint16_t r = voiceIn->lfoRandom;
                            uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5) ) & 1;
                            voiceIn->lfoRandom = (r >> 1) | (bit << 15);
                        }
                        lfoSample = voiceIn->lfoRandom & 0xFF;
                        break;
                    }
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
                switch (waveType)
                {
                    case WaveTypeSawtooth:
                    {
                        sample = accu16;
                        break;
                    }
                    case WaveTypePulse:
                    {
                        sample = ((accu16 >> 8) > pulseWidth) ? 0xFFFF : 0x0000;
                        break;
                    }
                    case WaveTypeTriangle:
                    {
                        sample = ((accu16 & 0x8000) ? ~(accu16 << 1) : (accu16 << 1));
                        break;
                    }
                    case WaveTypeNoise:
                    {
                        if ((accu16 & 0x1000) != (accu16Last & 0x1000))
                        {
                            uint16_t r = voiceIn->noiseRandom;
                            uint16_t bit = ((r >> 0) ^ (r >> 2) ^ (r >> 3) ^ (r >> 5) ) & 1;
                            voiceIn->noiseRandom = (r >> 1) | (bit << 15);
                        }
                        sample = voiceIn->noiseRandom & 0xFFFF;
                        break;
                    }
                }
                
                // --- TIMEOUT ---
                
                if (voice->attr.timeout)
                {
                    voiceIn->timeoutCounter -= 60.0 / outputFrequency;
                    if (voiceIn->timeoutCounter <= 0.0)
                    {
                        voiceIn->timeoutCounter = 0.0;
                        voice->status.gate = 0;
                    }
                }
                
                // --- ENVELOPE GENERATOR ---
                
                if (!voice->status.gate)
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
                
                // output peak to system registers
                lifeRegisters->voices[v].peak = volume;
                
                int16_t voiceSample = (((int32_t)(sample - 0x7FFF)) * volume) >> 10; // 8 bit for volume, 2 bit for global
                if (voice->status.mix & 0x01)
                {
                    leftOutput += voiceSample;
                }
                if (voice->status.mix & 0x02)
                {
                    rightOutput += voiceSample;
                }
            }
            
            // filter
            
            int32_t *filterBufferL = internals->filterBuffer[0];
            int32_t *filterBufferR = internals->filterBuffer[1];
            
            for (int f = AUDIO_FILTER_BUFFER_SIZE - 1; f > 0; f--)
            {
                filterBufferL[f] = filterBufferL[f - 1];
                filterBufferR[f] = filterBufferR[f - 1];
            }
            filterBufferL[0] = leftOutput;
            filterBufferR[0] = rightOutput;
            
            leftOutput  = ((filterBufferL[0] >> 4) + (filterBufferL[1] >> 1) + (filterBufferL[2] >> 4));
            rightOutput = ((filterBufferR[0] >> 4) + (filterBufferR[1] >> 1) + (filterBufferR[2] >> 4));
        }
        
        stereoOutput[i++] = leftOutput;
        stereoOutput[i++] = rightOutput;
    }
}
