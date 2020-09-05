//
// Copyright 2016-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#ifndef audio_chip_h
#define audio_chip_h

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define NUM_VOICES 4
#define NUM_AUDIO_BUFFERS 6
#define AUDIO_FILTER_BUFFER_SIZE 3

// audio output channels for stereo
#define NUM_CHANNELS 2

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

union VoiceStatus {
    struct {
        uint8_t volume:4;
        uint8_t mix:2;
        uint8_t init:1;
        uint8_t gate:1;
    };
    uint8_t value;
};

union VoiceAttributes {
    struct {
        uint8_t pulseWidth:4;
        uint8_t wave:2;
        uint8_t timeout:1;
    };
    uint8_t value;
};

enum LFOWaveType {
    LFOWaveTypeTriangle,
    LFOWaveTypeSawtooth,
    LFOWaveTypeSquare,
    LFOWaveTypeRandom
};

union LFOAttributes {
    struct {
        uint8_t wave:2;
        uint8_t invert:1;
        uint8_t envMode:1;
        uint8_t trigger:1;
    };
    uint8_t value;
};

struct Voice {
    uint8_t frequencyLow;
    uint8_t frequencyHigh;
    union VoiceStatus status;
    uint8_t peak;
    union VoiceAttributes attr;
    uint8_t length;
    struct {
        uint8_t envA:4;
        uint8_t envD:4;
    };
    struct {
        uint8_t envS:4;
        uint8_t envR:4;
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
    uint8_t reserved2;
};

struct AudioRegisters {
    struct Voice voices[NUM_VOICES];
};

struct VoiceInternals {
    double accumulator;
    uint16_t noiseRandom;
    double envCounter;
    enum EnvState envState;
    double lfoAccumulator;
    bool lfoHold;
    uint16_t lfoRandom;
    double timeoutCounter;
};

struct AudioInternals {
    struct VoiceInternals voices[NUM_VOICES];
    struct AudioRegisters buffers[NUM_AUDIO_BUFFERS];
    int readBufferIndex;
    int writeBufferIndex;
    bool audioEnabled;
    int32_t filterBuffer[NUM_CHANNELS][AUDIO_FILTER_BUFFER_SIZE];
};

void audio_reset(struct Core *core);
void audio_bufferRegisters(struct Core *core);
void audio_renderAudio(struct Core *core, int16_t *output, int numSamples, int outputFrequency, int volume);

#endif /* audio_chip_h */
