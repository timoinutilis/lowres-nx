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

#ifndef cmd_maths_h
#define cmd_maths_h

#include <stdio.h>
#include "value.h"

struct Core;

struct TypedValue fnc_math0(struct Core *core);
struct TypedValue fnc_math1(struct Core *core);
struct TypedValue fnc_math2(struct Core *core);
enum ErrorCode cmd_RANDOMIZE(struct Core *core);
struct TypedValue fnc_RND(struct Core *core);
enum ErrorCode cmd_ADD(struct Core *core);
enum ErrorCode cmd_INC_DEC(struct Core *core);

#endif /* cmd_maths_h */
