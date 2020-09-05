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
enum ErrorCode cmd_EXIT(struct Core *core);
enum ErrorCode cmd_SYSTEM(struct Core *core);

#endif /* cmd_control_h */
