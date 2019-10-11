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

#ifndef cmd_screen_h
#define cmd_screen_h

#include <stdio.h>
#include "error.h"
#include "value.h"

struct Core;

enum ErrorCode cmd_PALETTE(struct Core *core);
enum ErrorCode cmd_SCROLL(struct Core *core);
enum ErrorCode cmd_DISPLAY(struct Core *core);
enum ErrorCode cmd_SPRITE_VIEW(struct Core *core);
enum ErrorCode cmd_BG_VIEW(struct Core *core);
enum ErrorCode cmd_CELL_SIZE(struct Core *core);
struct TypedValue fnc_COLOR(struct Core *core);
struct TypedValue fnc_screen0(struct Core *core);
struct TypedValue fnc_SCROLL_X_Y(struct Core *core);

#endif /* cmd_screen_h */
