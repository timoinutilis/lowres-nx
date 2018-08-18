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

union VoiceAttributes {
    struct {
        uint8_t wave:2;
        uint8_t mixL:1;
        uint8_t mixR:1;
    };
    uint8_t value;
};

struct Voice {
    uint8_t frequencyLow;
    uint8_t frequencyHigh;
    uint8_t volume;
    uint8_t pulseWidth;
    union VoiceAttributes attr;
};

struct VoiceInternals {
    double accumulator;
    uint16_t noiseRandom;
};

struct AudioRegisters {
    struct Voice voices[NUM_VOICES];
};

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
    uint32_t accumulatorError;
};

void audio_reset(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency);

#endif /* audio_chip_h */
