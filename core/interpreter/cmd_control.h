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

#ifndef cmd_control_h
#define cmd_control_h

#include <stdio.h>
#include "error.h"

struct LowResCore;

enum ErrorCode cmd_END(struct LowResCore *core);
enum ErrorCode cmd_IF(struct LowResCore *core);
enum ErrorCode cmd_ELSE(struct LowResCore *core);
enum ErrorCode cmd_ENDIF(struct LowResCore *core);
enum ErrorCode cmd_FOR(struct LowResCore *core);
enum ErrorCode cmd_NEXT(struct LowResCore *core);
enum ErrorCode cmd_GOTO(struct LowResCore *core);
enum ErrorCode cmd_GOSUB(struct LowResCore *core);
enum ErrorCode cmd_RETURN(struct LowResCore *core);
enum ErrorCode cmd_WAIT(struct LowResCore *core);
enum ErrorCode cmd_ON(struct LowResCore *core);
enum ErrorCode cmd_DO(struct LowResCore *core);
enum ErrorCode cmd_LOOP(struct LowResCore *core);
enum ErrorCode cmd_EXIT(struct LowResCore *core);

#endif /* cmd_control_h */
