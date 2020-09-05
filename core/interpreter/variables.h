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

#ifndef variables_h
#define variables_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "interpreter_config.h"
#include "value.h"

#define SUB_LEVEL_GLOBAL -1

struct Core;
struct Interpreter;

struct SimpleVariable {
    int symbolIndex;
    int8_t subLevel;
    int8_t isReference:1;
    enum ValueType type;
    union Value v;
};

struct ArrayVariable {
    int symbolIndex;
    int8_t subLevel;
    int8_t isReference:1;
    enum ValueType type;
    int numDimensions;
    int dimensionSizes[MAX_ARRAY_DIMENSIONS];
    int numValues;
    union Value *values;
};

struct SimpleVariable *var_getSimpleVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel);
struct SimpleVariable *var_createSimpleVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, enum ValueType type, union Value *valueReference);
void var_freeSimpleVariables(struct Interpreter *interpreter, int minSubLevel);

struct ArrayVariable *var_getArrayVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel);
union Value *var_getArrayValue(struct Interpreter *interpreter, struct ArrayVariable *variable, int *indices);
struct ArrayVariable *var_dimVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int numDimensions, int *dimensionSizes);
struct ArrayVariable *var_createArrayVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, struct ArrayVariable *arrayReference);
void var_freeArrayVariables(struct Interpreter *interpreter, int minSubLevel);

#endif /* variables_h */
