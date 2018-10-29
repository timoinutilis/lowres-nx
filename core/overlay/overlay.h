//
// Copyright 2017-2018 Timo Kloss
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

#ifndef overlay_h
#define overlay_h

#include <stdio.h>
#include <stdbool.h>
#include "video_chip.h"
#include "overlay_data.h"
#include "text_lib.h"

struct Core;

struct Overlay {
    struct Plane plane;
    struct TextLib textLib;
    int timer;
    int messageTimer;
};

void overlay_init(struct Core *core);
void overlay_reset(struct Core *core);
void overlay_updateState(struct Core *core);
void overlay_message(struct Core *core, const char *message);
void overlay_draw(struct Core *core, bool ingame);

#endif /* overlay_h */
