//
// Copyright 2017 Timo Kloss
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

#ifndef token_h
#define token_h

#include <stdio.h>
#include "rcstring.h"

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
    
    TokenABS,
    TokenASC,
    TokenATN,
    TokenCHR,
    TokenCOS,
    TokenEXP,
    TokenHEX,
    TokenINSTR,
    TokenINT,
    TokenLEFT,
    TokenLEN,
    TokenLOG,
    TokenMAX,
    TokenMID,
    TokenMIN,
    TokenRIGHT,
    TokenRND,
    TokenSGN,
    TokenSIN,
    TokenSQR,
    TokenSTR,
    TokenTAN,
    TokenVAL,
    
    Token_count
};

struct Token {
    enum TokenType type;
    union {
        float floatValue;
        struct RCString *stringValue;
        int symbolIndex;
        struct Token *jumpToken;
    };
};

extern const char *TokenStrings[];

#endif /* token_h */
