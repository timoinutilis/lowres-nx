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

#ifndef cmd_strings_h
#define cmd_strings_h

#include <stdio.h>
#include "value.h"

struct Core;

struct TypedValue fnc_ASC(struct Core *core);
struct TypedValue fnc_BIN_HEX(struct Core *core);
struct TypedValue fnc_CHR(struct Core *core);
struct TypedValue fnc_INKEY(struct Core *core);
struct TypedValue fnc_INSTR(struct Core *core);
struct TypedValue fnc_LEFTStr_RIGHTStr(struct Core *core);
struct TypedValue fnc_LEN(struct Core *core);
struct TypedValue fnc_MID(struct Core *core);
struct TypedValue fnc_STR(struct Core *core);
struct TypedValue fnc_VAL(struct Core *core);

enum ErrorCode cmd_LEFT_RIGHT(struct Core *core);
enum ErrorCode cmd_MID(struct Core *core);

#endif /* cmd_strings_h */
