//
// Copyright 2017-2019 Timo Kloss
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
