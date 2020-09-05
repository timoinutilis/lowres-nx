//
// Copyright 2016-2017 Timo Kloss
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
