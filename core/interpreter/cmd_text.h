//
// Copyright 2017-2020 Timo Kloss
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
struct TypedValue fnc_CURSOR(struct Core *core);
enum ErrorCode cmd_CLW(struct Core *core);
enum ErrorCode cmd_TRACE(struct Core *core);

#endif /* cmd_text_h */
