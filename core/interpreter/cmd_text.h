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

#ifndef cmd_text_h
#define cmd_text_h

#include <stdio.h>
#include <stdbool.h>
#include "error.h"

struct Core;

enum ErrorCode cmd_PRINT(struct Core *core);
enum ErrorCode cmd_INPUT(struct Core *core);
enum ErrorCode cmd_endINPUT(struct Core *core);
enum ErrorCode cmd_TEXT(struct Core *core);
enum ErrorCode cmd_NUMBER(struct Core *core);
enum ErrorCode cmd_CLS(struct Core *core);
enum ErrorCode cmd_WINDOW(struct Core *core);
enum ErrorCode cmd_FONT(struct Core *core);
enum ErrorCode cmd_LOCATE(struct Core *core);
enum ErrorCode cmd_CLW(struct Core *core);
enum ErrorCode cmd_TRACE(struct Core *core);

#endif /* cmd_text_h */
