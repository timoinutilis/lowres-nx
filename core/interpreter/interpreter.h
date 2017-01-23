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

#define MAX_TOKENS 1024

struct LowResCore;

enum TokenType {
    TokenUndefined,
    
    TokenIdentifier,
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

enum ValueType {
    ValueNull,
    ValueError,
    ValueFloat,
    ValueString
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
    ErrorExpectedThen
};

struct Value {
    enum ValueType type;
    union {
        float floatValue;
        enum ErrorCode errorCode;
    };
};

struct Token {
    enum TokenType type;
    union {
        /* TokenFloat */    float floatValue;
    };
};

struct Interpreter {
    int numTokens;
    struct Token tokens[MAX_TOKENS];
    struct Token *pc;
};

enum ErrorCode LRC_tokenizeProgram(struct LowResCore *core, const char *sourceCode);
enum ErrorCode LRC_runProgram(struct LowResCore *core);
enum ErrorCode LRC_runCommand(struct LowResCore *core);

#endif /* interpreter_h */
