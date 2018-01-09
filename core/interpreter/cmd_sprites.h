//
// Copyright 2017 Timo Kloss
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

#ifndef cmd_sprites_h
#define cmd_sprites_h

#include <stdio.h>
#include "error.h"
#include "value.h"

struct Core;

enum ErrorCode cmd_SPRITE(struct Core *core);
enum ErrorCode cmd_SPRITE_A(struct Core *core);
enum ErrorCode cmd_SPRITE_OFF(struct Core *core);
struct TypedValue fnc_SPRITE(struct Core *core);
struct TypedValue fnc_SPRITE_HIT(struct Core *core);
struct TypedValue fnc_HIT(struct Core *core);

#endif /* cmd_sprites_h */
