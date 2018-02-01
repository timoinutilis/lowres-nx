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

#ifndef labels_h
#define labels_h

#include <stdio.h>

struct Interpreter;
struct Token;

enum LabelType {
    LabelTypeIF,
    LabelTypeELSE,
    LabelTypeELSEIF,
    LabelTypeFOR,
    LabelTypeFORVar,
    LabelTypeFORLimit,
    LabelTypeGOSUB,
    LabelTypeDO,
    LabelTypeREPEAT,
    LabelTypeWHILE,
    LabelTypeSUB,
    LabelTypeCALL,
    LabelTypeONCALL,
};

struct LabelStackItem {
    enum LabelType type;
    struct Token *token;
};

enum ErrorCode lab_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token);
struct LabelStackItem *lab_popLabelStackItem(struct Interpreter *interpreter);
struct LabelStackItem *lab_peekLabelStackItem(struct Interpreter *interpreter);

#endif /* labels_h */
