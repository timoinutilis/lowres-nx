//
// Copyright 2016-2017 Timo Kloss
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

#ifndef tokenizer_h
#define tokenizer_h

#include <stdio.h>
#include "interpreter_config.h"
#include "token.h"

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct JumpLabelItem {
    int symbolIndex;
    struct Token *token;
};

struct SubItem {
    int symbolIndex;
    struct Token *token;
};

struct Tokenizer
{
    struct Token tokens[MAX_TOKENS];
    int numTokens;
    struct Symbol symbols[MAX_SYMBOLS];
    int numSymbols;
    
    struct JumpLabelItem jumpLabelItems[MAX_JUMP_LABEL_ITEMS];
    int numJumpLabelItems;
    struct SubItem subItems[MAX_SUB_ITEMS];
    int numSubItems;
};

struct CoreError tok_tokenizeProgram(struct Tokenizer *tokenizer, const char *sourceCode);
struct CoreError tok_tokenizeUppercaseProgram(struct Tokenizer *tokenizer, const char *sourceCode);
void tok_freeTokens(struct Tokenizer *tokenizer);
struct JumpLabelItem *tok_getJumpLabel(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setJumpLabel(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);
struct SubItem *tok_getSub(struct Tokenizer *tokenizer, int symbolIndex);
enum ErrorCode tok_setSub(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token);

#endif /* tokenizer_h */
