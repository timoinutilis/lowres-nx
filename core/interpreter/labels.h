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

#ifndef labels_h
#define labels_h

#include <stdio.h>

struct Interpreter;
struct Token;

enum LabelType {
    LabelTypeIF,
    LabelTypeELSE,
    LabelTypeFOR,
    LabelTypeFORVar,
    LabelTypeFORLimit,
    LabelTypeGOSUB
};

struct LabelStackItem {
    enum LabelType type;
    struct Token *token;
};

struct JumpLabelItem {
    int symbolIndex;
    struct Token *token;
};

enum ErrorCode LRC_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token);
struct LabelStackItem *LRC_popLabelStackItem(struct Interpreter *interpreter);
struct LabelStackItem *LRC_peekLabelStackItem(struct Interpreter *interpreter);
struct JumpLabelItem *LRC_getJumpLabel(struct Interpreter *interpreter, int symbolIndex);
enum ErrorCode LRC_setJumpLabel(struct Interpreter *interpreter, int symbolIndex, struct Token *token);

#endif /* labels_h */
