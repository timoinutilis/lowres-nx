//
// Copyright 2017 Timo Kloss
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
struct LabelStackItem *lab_searchLabelStackItem(struct Interpreter *interpreter, enum LabelType types[], int numTypes);

#endif /* labels_h */
