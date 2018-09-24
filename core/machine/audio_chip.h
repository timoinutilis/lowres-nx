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

#ifndef audio_chip_h
#define audio_chip_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define NUM_VOICES 4
#define NUM_AUDIO_BUFFERS 4

struct Core;

enum WaveType {
    WaveTypeSawtooth,
    WaveTypeTriangle,
    WaveTypePulse,
    WaveTypeNoise
};

enum EnvState {
    EnvStateAttack,
    EnvStateDecay,
    EnvStateRelease
};

union VoiceAttributes {
    struct {
        uint8_t wave:2;
        uint8_t mixL:1;
        uint8_t mixR:1;
        uint8_t timeout:1;
        uint8_t reserved:1;
        uint8_t init:1;
        uint8_t gate:1;
    };
    uint8_t value;
};

union LFOAttributes {
    struct {
        uint8_t wave:1;
        uint8_t invert:1;
        uint8_t envMode:1;
        uint8_t trigger:1;
    };
    uint8_t value;
};

struct Voice {
    uint8_t frequencyLow;
    uint8_t frequencyHigh;
    struct {
        uint8_t volume:4;
        uint8_t pulseWidth:4;
    };
    union VoiceAttributes attr;
    uint8_t length;
    struct {
        uint8_t envD:4;
        uint8_t envA:4;
    };
    struct {
        uint8_t envR:4;
        uint8_t envS:4;
    };
    union LFOAttributes lfoAttr;
    struct {
        uint8_t lfoFrequency:4;
        uint8_t lfoOscAmount:4;
    };
    struct {
        uint8_t lfoVolAmount:4;
        uint8_t lfoPWAmount:4;
    };
};

union AudioAttributes {
    struct {
        uint8_t audioEnabled:1;
    };
    uint8_t value;
};

struct AudioRegisters {
    struct Voice voices[NUM_VOICES];
    union AudioAttributes attr;
};

struct VoiceInternals {
    double accumulator;
    uint16_t noiseRandom;
    double envCounter;
    enum EnvState envState;
    double lfoAccumulator;
    bool lfoHold;
    double timeoutCounter;
};

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
    struct AudioRegisters buffers[NUM_AUDIO_BUFFERS];
    int readBufferIndex;
    int writeBufferIndex;
};

void audio_reset(struct Core *core);
void audio_bufferRegisters(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency);

#endif /* audio_chip_h */
