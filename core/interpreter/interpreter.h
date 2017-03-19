//
// Copyright 2016-2017 Timo Kloss
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

#ifndef interpreter_h
#define interpreter_h

#include <stdio.h>
#include <stdbool.h>
#include "token.h"
#include "error.h"
#include "value.h"
#include "text_lib.h"

#define MAX_TOKENS 1024
#define MAX_SYMBOLS 128
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 128
#define MAX_SIMPLE_VARIABLES 128
#define MAX_ARRAY_VARIABLES 128
#define SYMBOL_NAME_SIZE 11
#define MAX_ARRAY_DIMENSIONS 8
#define MAX_ARRAY_SIZE 32768

struct LowResCore;

enum Pass {
    PassPrepare,
    PassRun
};

enum LabelType {
    LabelTypeIF,
    LabelTypeELSE,
    LabelTypeFOR,
    LabelTypeFORVar,
    LabelTypeFORLimit,
};

struct LabelStackItem {
    enum LabelType type;
    struct Token *token;
};

struct JumpLabelItem {
    int symbolIndex;
    struct Token *token;
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

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct SimpleVariable {
    int symbolIndex;
    enum ValueType type;
    union Value v;
};

struct ArrayVariable {
    int symbolIndex;
    enum ValueType type;
    int numDimensions;
    int dimensionSizes[MAX_ARRAY_DIMENSIONS];
    union Value *values;
};

struct Interpreter {
    enum Pass pass;
    struct Token tokens[MAX_TOKENS];
    int numTokens;
    struct Symbol symbols[MAX_SYMBOLS];
    int numSymbols;
    struct LabelStackItem labelStackItems[MAX_LABEL_STACK_ITEMS];
    int numLabelStackItems;
    bool isSingleLineIf;
    struct JumpLabelItem jumpLabelItems[MAX_JUMP_LABEL_ITEMS];
    int numJumpLabelItems;
    struct Token *pc;
    struct SimpleVariable simpleVariables[MAX_SIMPLE_VARIABLES];
    int numSimpleVariables;
    struct ArrayVariable arrayVariables[MAX_ARRAY_VARIABLES];
    int numArrayVariables;
    struct RCString *nullString;
    
    struct TextLib textLib;
};

enum ErrorCode LRC_compileProgram(struct LowResCore *core, const char *sourceCode);
enum ErrorCode LRC_runProgram(struct LowResCore *core);
void LRC_freeProgram(struct LowResCore *core);

union Value *LRC_readVariable(struct LowResCore *core, enum ValueType *type, enum ErrorCode *errorCode);
struct ArrayVariable *LRC_getArrayVariable(struct Interpreter *interpreter, int symbolIndex);
struct ArrayVariable *LRC_dimVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int numDimensions, int *dimensionSizes);
struct TypedValue LRC_evaluateExpression(struct LowResCore *core, enum TypeClass typeClass);
struct TypedValue LRC_makeError(enum ErrorCode errorCode);
int LRC_isEndOfCommand(struct Interpreter *interpreter);
enum ErrorCode LRC_endOfCommand(struct Interpreter *interpreter);
void LRC_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token);
struct LabelStackItem *LRC_popLabelStackItem(struct Interpreter *interpreter);
struct LabelStackItem *LRC_peekLabelStackItem(struct Interpreter *interpreter);
struct JumpLabelItem *LRC_getJumpLabel(struct Interpreter *interpreter, int symbolIndex);
enum ErrorCode LRC_setJumpLabel(struct Interpreter *interpreter, int symbolIndex, struct Token *token);

#endif /* interpreter_h */
