//
// Copyright 2017 Timo Kloss
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
