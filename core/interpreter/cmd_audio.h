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

#ifndef cmd_audio_h
#define cmd_audio_h

#include <stdio.h>
#include "error.h"
#include "value.h"

struct Core;

enum ErrorCode cmd_SOUND(struct Core *core);
//enum ErrorCode cmd_SOUND_COPY(struct Core *core);
enum ErrorCode cmd_VOLUME(struct Core *core);
enum ErrorCode cmd_ENVELOPE(struct Core *core);
enum ErrorCode cmd_LFO(struct Core *core);
enum ErrorCode cmd_LFO_A(struct Core *core);
enum ErrorCode cmd_LFO_WAVE(struct Core *core);
enum ErrorCode cmd_PLAY(struct Core *core);
enum ErrorCode cmd_STOP(struct Core *core);
enum ErrorCode cmd_MUSIC(struct Core *core);
enum ErrorCode cmd_TRACK(struct Core *core);
enum ErrorCode cmd_SOUND_SOURCE(struct Core *core);
struct TypedValue fnc_MUSIC(struct Core *core);

#endif /* cmd_audio_h */
