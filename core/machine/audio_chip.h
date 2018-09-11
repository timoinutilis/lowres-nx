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
        uint8_t lfoWave:1;
        uint8_t lfoEnvMode:1;
        uint8_t lfoTrigger:1;
        uint8_t gate:1;
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
    struct {
        uint8_t envD:4;
        uint8_t envA:4;
    };
    struct {
        uint8_t envR:4;
        uint8_t envS:4;
    };
    struct {
        uint8_t lfoFrequency:4;
        uint8_t lfoOscAmount:3;
        uint8_t lfoOscSign:1;
    };
    struct {
        uint8_t lfoVolAmount:3;
        uint8_t lfoVolSign:1;
        uint8_t lfoPWAmount:3;
        uint8_t lfoPWSign:1;
    };
};

struct VoiceInternals {
    double accumulator;
    uint16_t noiseRandom;
    double envCounter;
    enum EnvState envState;
    double lfoAccumulator;
    bool lfoHold;
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

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
};

void audio_reset(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency);
void audio_onVoiceAttrChange(struct Core *core, int index);

#endif /* audio_chip_h */
