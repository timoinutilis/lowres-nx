//
// Copyright 2017-2019 Timo Kloss
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

#ifndef cmd_background_h
#define cmd_background_h

#include <stdio.h>
#include "error.h"
#include "value.h"

struct Core;

enum ErrorCode cmd_BG(struct Core *core);
enum ErrorCode cmd_BG_SOURCE(struct Core *core);
enum ErrorCode cmd_BG_COPY(struct Core *core);
enum ErrorCode cmd_BG_SCROLL(struct Core *core);
enum ErrorCode cmd_ATTR(struct Core *core);
enum ErrorCode cmd_PAL(struct Core *core);
enum ErrorCode cmd_FLIP(struct Core *core);
enum ErrorCode cmd_PRIO(struct Core *core);
enum ErrorCode cmd_BG_FILL(struct Core *core);
enum ErrorCode cmd_BG_TINT(struct Core *core);
enum ErrorCode cmd_CELL(struct Core *core);
struct TypedValue fnc_CELL(struct Core *core);
enum ErrorCode cmd_MCELL(struct Core *core);
struct TypedValue fnc_MCELL(struct Core *core);
enum ErrorCode cmd_TINT(struct Core *core);

#endif /* cmd_background_h */
