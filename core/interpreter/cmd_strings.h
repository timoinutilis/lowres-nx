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
