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

#define NUM_VOICES 4

struct Core;

enum WaveType {
    WaveTypeSawtooth,
    WaveTypeTriangle,
    WaveTypePulse,
    WaveTypeNoise
};

struct Voice {
    uint8_t frequencyLow;   // 0
    uint8_t frequencyHigh;  // 1
    uint8_t volume;         // 2
    uint8_t wave;           // 3
    uint8_t pulseWidth;     // 4
};

struct VoiceInternals {
    uint16_t accumulator;
    uint16_t noiseRandom;
};

union Mixer {
    struct {
        uint8_t v0R:1;
        uint8_t v1R:1;
        uint8_t v2R:1;
        uint8_t v3R:1;
        uint8_t v0L:1;
        uint8_t v1L:1;
        uint8_t v2L:1;
        uint8_t v3L:1;
    };
    uint8_t value;
};

struct AudioRegisters {
    struct Voice voices[NUM_VOICES];
    union Mixer mixer;
};

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
    uint32_t accumulatorError;
};

void audio_reset(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency);

#endif /* audio_chip_h */
