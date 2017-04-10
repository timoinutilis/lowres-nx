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

#include "labels.h"
#include "interpreter.h"

enum ErrorCode lab_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token)
{
    if (interpreter->numLabelStackItems >= MAX_LABEL_STACK_ITEMS) return ErrorStackOverflow;
    struct LabelStackItem *item = &interpreter->labelStackItems[interpreter->numLabelStackItems];
    item->type = type;
    item->token = token;
    interpreter->numLabelStackItems++;
    return ErrorNone;
}

struct LabelStackItem *lab_popLabelStackItem(struct Interpreter *interpreter)
{
    if (interpreter->numLabelStackItems > 0)
    {
        interpreter->numLabelStackItems--;
        return &interpreter->labelStackItems[interpreter->numLabelStackItems];
    }
    return NULL;
}

struct LabelStackItem *lab_peekLabelStackItem(struct Interpreter *interpreter)
{
    if (interpreter->numLabelStackItems > 0)
    {
        return &interpreter->labelStackItems[interpreter->numLabelStackItems - 1];
    }
    return NULL;
}

struct JumpLabelItem *lab_getJumpLabel(struct Interpreter *interpreter, int symbolIndex)
{
    struct JumpLabelItem *item;
    for (int i = 0; i < interpreter->numJumpLabelItems; i++)
    {
        item = &interpreter->jumpLabelItems[i];
        if (item->symbolIndex == symbolIndex)
        {
            return item;
        }
    }
    return NULL;
}

enum ErrorCode lab_setJumpLabel(struct Interpreter *interpreter, int symbolIndex, struct Token *token)
{
    if (lab_getJumpLabel(interpreter, symbolIndex) != NULL)
    {
        return ErrorLabelAlreadyDefined;
    }
    if (interpreter->numJumpLabelItems >= MAX_JUMP_LABEL_ITEMS)
    {
        return ErrorTooManyLabels;
    }
    struct JumpLabelItem *item = &interpreter->jumpLabelItems[interpreter->numJumpLabelItems];
    item->symbolIndex = symbolIndex;
    item->token = token;
    interpreter->numJumpLabelItems++;
    return ErrorNone;
}
