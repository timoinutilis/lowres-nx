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

#ifndef cmd_memory_h
#define cmd_memory_h

#include <stdio.h>
#include "error.h"

struct Core;

struct TypedValue fnc_PEEK(struct Core *core);
enum ErrorCode cmd_POKE(struct Core *core);
enum ErrorCode cmd_FILL(struct Core *core);
enum ErrorCode cmd_COPY(struct Core *core);
struct TypedValue fnc_ROM_SIZE(struct Core *core);
enum ErrorCode cmd_ROL_ROR(struct Core *core);

#endif /* cmd_memory_h */
