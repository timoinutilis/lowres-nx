//
// Copyright 2016 Timo Kloss
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
#include "text_lib.h"

#define MAX_TOKENS 1024
#define MAX_SYMBOLS 128
#define SYMBOL_NAME_SIZE 11
#define VARIABLES_STACK_SIZE 4096

struct LowResCore;

extern const char *ErrorStrings[];

enum TokenType {
    TokenUndefined,
    
    TokenIdentifier,
    TokenStringIdentifier,
    TokenLabel,
    TokenFloat,
    TokenString,
    
    TokenColon,
    TokenComma,
    TokenSemicolon,
    TokenEol,
    
    TokenEq,
    TokenGrEq,
    TokenLeEq,
    TokenUneq,
    TokenGr,
    TokenLe,
    TokenBracketOpen,
    TokenBracketClose,
    TokenPlus,
    TokenMinus,
    TokenMul,
    TokenDiv,
    TokenPow,
    
    TokenAND,
    TokenDATA,
    TokenDIM,
    TokenELSE,
    TokenEND,
    TokenFOR,
    TokenGOSUB,
    TokenGOTO,
    TokenIF,
    TokenINPUT,
    TokenLET,
    TokenMOD,
    TokenNEXT,
    TokenNOT,
    TokenON,
    TokenOR,
    TokenPEEK,
    TokenPOKE,
    TokenPRINT,
    TokenRANDOMIZE,
    TokenREAD,
    TokenREM,
    TokenRESTORE,
    TokenRETURN,
    TokenSTEP,
    TokenTHEN,
    TokenTO,
    TokenXOR,
    
    Token_count
};

enum ErrorCode {
    ErrorNone,
    ErrorTooManyTokens,
    ErrorExpectedEndOfString,
    ErrorUnexpectedCharacter,
    ErrorSyntax,
    ErrorEndOfProgram,
    ErrorUnexpectedToken,
    ErrorExpectedEndOfLine,
    ErrorExpectedThen,
    ErrorExpectedEqualSign,
    ErrorExpectedVariableIdentifier,
    ErrorExpectedRightParenthesis,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory
};

enum ValueType {
    ValueNull,
    ValueError,
    ValueFloat,
    ValueString
};

union Value {
    float floatValue;
    const char *stringValue;
    enum ErrorCode errorCode;
};

struct TypedValue {
    enum ValueType type;
    union Value v;
};

struct Token {
    enum TokenType type;
    union {
        float floatValue;
        const char *stringValue;
        uint16_t symbolIndex;
    };
};

struct Symbol {
    char name[SYMBOL_NAME_SIZE];
};

struct SimpleVariable {
    uint16_t symbolIndex;
    union Value v;
};

struct Interpreter {
    int numTokens;
    int numSymbols;
    struct Token tokens[MAX_TOKENS];
    struct Symbol symbols[MAX_SYMBOLS];
    struct Token *pc;
    uint8_t variablesStack[VARIABLES_STACK_SIZE];
    struct SimpleVariable *simpleVariablesEnd;
    
    struct TextLib textLib;
};

enum ErrorCode LRC_tokenizeProgram(struct LowResCore *core, const char *sourceCode);
enum ErrorCode LRC_runProgram(struct LowResCore *core);

#endif /* interpreter_h */
