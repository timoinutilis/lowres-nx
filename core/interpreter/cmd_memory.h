//
// Copyright 2017 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef cmd_memory_h
#define cmd_memory_h

#include <stdio.h>
#include "error.h"

struct LowResCore;

struct TypedValue fnc_PEEK(struct LowResCore *core);
enum ErrorCode cmd_POKE(struct LowResCore *core);
enum ErrorCode cmd_FILL(struct LowResCore *core);
enum ErrorCode cmd_COPY(struct LowResCore *core);
struct TypedValue fnc_STARTLENGTH(struct LowResCore *core);

#endif /* cmd_memory_h */
