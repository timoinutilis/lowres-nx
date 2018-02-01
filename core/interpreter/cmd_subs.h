//
// Copyright 2018 Timo Kloss
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

#ifndef cmd_subs_h
#define cmd_subs_h

#include <stdio.h>
#include "error.h"

struct Core;

enum ErrorCode cmd_CALL(struct Core *core);
enum ErrorCode cmd_SUB(struct Core *core);
enum ErrorCode cmd_END_SUB(struct Core *core);
//enum ErrorCode cmd_SHARED(struct Core *core);
enum ErrorCode cmd_GLOBAL(struct Core *core);
enum ErrorCode cmd_EXIT_SUB(struct Core *core);

#endif /* cmd_subs_h */
