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

#ifndef cmd_control_h
#define cmd_control_h

#include <stdio.h>
#include <stdbool.h>
#include "error.h"

struct Core;

enum ErrorCode cmd_END(struct Core *core);
enum ErrorCode cmd_IF(struct Core *core, bool isAfterBlockElse);
enum ErrorCode cmd_ELSE(struct Core *core);
enum ErrorCode cmd_END_IF(struct Core *core);
enum ErrorCode cmd_FOR(struct Core *core);
enum ErrorCode cmd_NEXT(struct Core *core);
enum ErrorCode cmd_GOTO(struct Core *core);
enum ErrorCode cmd_GOSUB(struct Core *core);
enum ErrorCode cmd_RETURN(struct Core *core);
enum ErrorCode cmd_WAIT(struct Core *core);
enum ErrorCode cmd_ON(struct Core *core);
enum ErrorCode cmd_DO(struct Core *core);
enum ErrorCode cmd_LOOP(struct Core *core);
enum ErrorCode cmd_REPEAT(struct Core *core);
enum ErrorCode cmd_UNTIL(struct Core *core);
enum ErrorCode cmd_WHILE(struct Core *core);
enum ErrorCode cmd_WEND(struct Core *core);
enum ErrorCode cmd_SYSTEM(struct Core *core);

#endif /* cmd_control_h */
