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
#include "config.h"
#include "token.h"
#include "error.h"
#include "value.h"
#include "labels.h"
#include "variables.h"
#include "data.h"
#include "big_endian.h"
#include "text_lib.h"

struct LowResCore;

enum Pass {
    PassPrepare,
    PassRun
};

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct RomDataEntry {
    BigEndianUInt16 start;
    BigEndianUInt16 length;
};

struct Interpreter {
    enum Pass pass;
    struct Token *pc;
    
    struct Token tokens[MAX_TOKENS];
    int numTokens;
    struct Symbol symbols[MAX_SYMBOLS];
    int numSymbols;
    
    struct LabelStackItem labelStackItems[MAX_LABEL_STACK_ITEMS];
    int numLabelStackItems;
    struct JumpLabelItem jumpLabelItems[MAX_JUMP_LABEL_ITEMS];
    int numJumpLabelItems;
    bool isSingleLineIf;
    
    struct SimpleVariable simpleVariables[MAX_SIMPLE_VARIABLES];
    int numSimpleVariables;
    struct ArrayVariable arrayVariables[MAX_ARRAY_VARIABLES];
    int numArrayVariables;
    struct RCString *nullString;
    
    struct Token *firstData;
    struct Token *lastData;
    struct Token *currentDataToken;
    struct Token *currentDataValueToken;
    
    struct TextLib textLib;
};

enum ErrorCode LRC_compileProgram(struct LowResCore *core, const char *sourceCode);
enum ErrorCode LRC_runProgram(struct LowResCore *core);
void LRC_freeProgram(struct LowResCore *core);

union Value *LRC_readVariable(struct LowResCore *core, enum ValueType *type, enum ErrorCode *errorCode);
struct TypedValue LRC_evaluateExpression(struct LowResCore *core, enum TypeClass typeClass);
int LRC_isEndOfCommand(struct Interpreter *interpreter);
enum ErrorCode LRC_endOfCommand(struct Interpreter *interpreter);
struct TypedValue LRC_makeError(enum ErrorCode errorCode);

#endif /* interpreter_h */
