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
