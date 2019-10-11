//
// Copyright 2017-2019 Timo Kloss
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
    
    // Signs
    TokenColon,
    TokenComma,
    TokenSemicolon,
    TokenApostrophe,
    TokenEol,
    
    // Operators
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
    TokenDivInt,
    TokenPow,
    TokenAND,
    TokenNOT,
    TokenOR,
    TokenXOR,
    TokenMOD,
    
    // Commands/Functions
    TokenABS,
    TokenACOS,
    TokenADD,
    TokenASC,
    TokenASIN,
    TokenATAN,
    TokenATTR,
    TokenBG,
    TokenBIN,
    TokenBUTTON,
    TokenCALL,
    TokenCELLA,
    TokenCELLC,
    TokenCELL,
    TokenCHAR,
    TokenCHR,
    TokenCLS,
    TokenCLW,
    TokenCOLOR,
    TokenCOPY,
    TokenCOS,
    TokenDATA,
    TokenDEC,
    TokenDIM,
    TokenDISPLAY,
    TokenDOWN,
    TokenDO,
    TokenELSE,
    TokenEND,
    TokenENVELOPE,
    TokenEXIT,
    TokenEXP,
    TokenFILE,
    TokenFILES,
    TokenFILL,
    TokenFLIP,
    TokenFONT,
    TokenFOR,
    TokenFSIZE,
    TokenGAMEPAD,
    TokenGLOBAL,
    TokenGOSUB,
    TokenGOTO,
    TokenHEX,
    TokenHCOS,
    TokenHIT,
    TokenHSIN,
    TokenHTAN,
    TokenIF,
    TokenINC,
    TokenINKEY,
    TokenINPUT,
    TokenINSTR,
    TokenINT,
    TokenKEYBOARD,
    TokenLEFTStr,
    TokenLEFT,
    TokenLEN,
    TokenLET,
    TokenLFOA,
    TokenLFO,
    TokenLOAD,
    TokenLOCATE,
    TokenLOG,
    TokenLOOP,
    TokenMAX,
    TokenMCELLA,
    TokenMCELLC,
    TokenMCELL,
    TokenMID,
    TokenMIN,
    TokenMUSIC,
    TokenNEXT,
    TokenNUMBER,
    TokenOFF,
    TokenON,
    TokenOPTIONAL,
    TokenPALETTE,
    TokenPAL,
    TokenPAUSE,
    TokenPEEKL,
    TokenPEEKW,
    TokenPEEK,
    TokenPI,
    TokenPLAY,
    TokenPOKEL,
    TokenPOKEW,
    TokenPOKE,
    TokenPRINT,
    TokenPRIO,
    TokenRANDOMIZE,
    TokenRASTER,
    TokenREAD,
    TokenREM,
    TokenREPEAT,
    TokenRESTORE,
    TokenRETURN,
    TokenRIGHTStr,
    TokenRIGHT,
    TokenRND,
    TokenROL,
    TokenROM,
    TokenROR,
    TokenSAVE,
    TokenSCROLLX,
    TokenSCROLLY,
    TokenSCROLL,
    TokenSGN,
    TokenSIN,
    TokenSIZE,
    TokenSOUND,
    TokenSOURCE,
    TokenSPRITEA,
    TokenSPRITEC,
    TokenSPRITEX,
    TokenSPRITEY,
    TokenSPRITE,
    TokenSQR,
    TokenSTEP,
    TokenSTOP,
    TokenSTR,
    TokenSUB,
    TokenSWAP,
    TokenSYSTEM,
    TokenTAN,
    TokenTAP,
    TokenTEXT,
    TokenTHEN,
    TokenTIMER,
    TokenTINT,
    TokenTOUCHSCREEN,
    TokenTOUCHX,
    TokenTOUCHY,
    TokenTOUCH,
    TokenTO,
    TokenTRACE,
    TokenTRACK,
    TokenUNTIL,
    TokenUP,
    TokenVAL,
    TokenVBL,
    TokenVIEW,
    TokenVOLUME,
    TokenWAIT,
    TokenWAVE,
    TokenWEND,
    TokenWHILE,
    TokenWINDOW,
    
    // Reserved Keywords
    Token_reserved,
    TokenANIM,
    TokenCLOSE,
    TokenDECLARE,
    TokenDEF,
    TokenFLASH,
    TokenFN,
    TokenFUNCTION,
    TokenLBOUND,
    TokenOPEN,
    TokenOUTPUT,
    TokenSHARED,
    TokenSTATIC,
    TokenTEMPO,
    TokenUBOUND,
    TokenVOICE,
    TokenWRITE,
    
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
    int sourcePosition;
};

extern const char *TokenStrings[];

#endif /* token_h */
