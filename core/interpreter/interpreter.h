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
#define SYMBOL_NAME_SIZE 11
#define VARIABLES_STACK_SIZE 4096

struct LowResCore;

enum Pass {
    PASS_PREPARE,
    PASS_RUN
};

struct LabelStackItem {
    enum TokenType type;
    struct Token *token;
};

struct TypedValue {
    enum ValueType type;
    union Value v;
};

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct SimpleVariable {
    uint16_t symbolIndex;
    union Value v;
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
    struct Token *pc;
    uint8_t variablesStack[VARIABLES_STACK_SIZE];
    struct SimpleVariable *simpleVariablesEnd;
    
    struct TextLib textLib;
};

enum ErrorCode LRC_compileProgram(struct LowResCore *core, const char *sourceCode);
enum ErrorCode LRC_runProgram(struct LowResCore *core);

union Value *LRC_readVariable(struct LowResCore *core, enum ErrorCode *errorCode);
struct TypedValue LRC_evaluateExpression(struct LowResCore *core);
int LRC_isEndOfCommand(struct Interpreter *interpreter);
enum ErrorCode LRC_endOfCommand(struct Interpreter *interpreter);
enum ErrorCode LRC_runCommand(struct LowResCore *core);
void LRC_pushLabelStackItem(struct Interpreter *interpreter, enum TokenType type, struct Token *token);
struct LabelStackItem *LRC_popLabelStackItem(struct Interpreter *interpreter);
struct LabelStackItem *LRC_peekLabelStackItem(struct Interpreter *interpreter);

#endif /* interpreter_h */
