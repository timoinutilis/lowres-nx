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

#ifndef value_h
#define value_h

#include <stdio.h>
#include "error.h"
#include "rcstring.h"

enum ValueType {
    ValueTypeNull,
    ValueTypeError,
    ValueTypeFloat,
    ValueTypeString
};

union Value {
    float floatValue;
    struct RCString *stringValue;
    union Value *reference;
    enum ErrorCode errorCode;
};

struct TypedValue {
    enum ValueType type;
    union Value v;
};

enum TypeClass {
    TypeClassAny,
    TypeClassNumeric,
    TypeClassString
};

extern union Value ValueDummy;

struct TypedValue val_makeError(enum ErrorCode errorCode);

#endif /* value_h */
