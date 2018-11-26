//
// Copyright 2018 Timo Kloss
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

#ifndef audio_lib_h
#define audio_lib_h

#include <stdio.h>

#define NUM_SOUNDS 16
#define NUM_PATTERNS 64
#define NUM_TRACKS 64

struct Core;

struct AudioLib {
    struct Core *core;
    int soundSourceAddress;
    int musicSourceAddress;
    int trackSourceAddress;
};

void audlib_play(struct AudioLib *lib, int voiceIndex, float pitch, int len, int sound);
void audlib_copySound(struct AudioLib *lib, int sound, int voiceIndex);
void audlib_playMusic(struct AudioLib *lib, int startPattern);
void audlib_playTrack(struct AudioLib *lib, int voiceIndex, int track);
void audlib_stopAll(struct AudioLib *lib);
void audlib_stopVoice(struct AudioLib *lib, int voiceIndex);

#endif /* audio_lib_h */
