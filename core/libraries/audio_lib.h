//
// Copyright 2018 Timo Kloss
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

#ifndef audio_lib_h
#define audio_lib_h

#include <stdio.h>
#include <stdbool.h>
#include "audio_chip.h"

#define NUM_SOUNDS 16
#define NUM_PATTERNS 64
#define NUM_TRACKS 64
#define NUM_TRACK_ROWS 32

struct Core;

struct ComposerPlayer {
    int sourceAddress;
    int index; // pattern for music, otherwise track
    int speed;
    int tick;
    int row;
    bool willBreak;
};

struct AudioLib {
    struct Core *core;
    int sourceAddress;
    
    struct ComposerPlayer musicPlayer;
    struct ComposerPlayer trackPlayers[NUM_VOICES];
};

void audlib_play(struct AudioLib *lib, int voiceIndex, float pitch, int len, int sound);
void audlib_copySound(struct AudioLib *lib, int sourceAddress, int sound, int voiceIndex);
void audlib_playMusic(struct AudioLib *lib, int startPattern);
void audlib_playTrack(struct AudioLib *lib, int track, int voiceIndex);
void audlib_stopAll(struct AudioLib *lib);
void audlib_stopVoice(struct AudioLib *lib, int voiceIndex);
void audlib_update(struct AudioLib *lib);

#endif /* audio_lib_h */
