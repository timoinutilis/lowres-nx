//
// Copyright 2017 Timo Kloss
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
    TokenASC,
    TokenATN,
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
    TokenDIM,
    TokenDISPLAY,
    TokenDOWN,
    TokenDO,
    TokenELSE,
    TokenEND,
    TokenEXIT,
    TokenEXP,
    TokenFILE,
    TokenFILES,
    TokenFILL,
    TokenFONT,
    TokenFOR,
    TokenGAMEPAD,
    TokenGLOBAL,
    TokenGOSUB,
    TokenGOTO,
    TokenHEX,
    TokenHIT,
    TokenIF,
    TokenINKEY,
    TokenINPUT,
    TokenINSTR,
    TokenINT,
    TokenKEYBOARD,
    TokenLEFTStr,
    TokenLEFT,
    TokenLEN,
    TokenLET,
    TokenLOAD,
    TokenLOCATE,
    TokenLOG,
    TokenLOOP,
    TokenMAX,
    TokenMID,
    TokenMIN,
    TokenNEXT,
    TokenNUMBER,
    TokenOFF,
    TokenON,
    TokenPALETTE,
    TokenPAUSE,
    TokenPEEK,
    TokenPI,
    TokenPOKE,
    TokenPRINT,
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
    TokenROM,
    TokenSAVE,
    TokenSCROLLX,
    TokenSCROLLY,
    TokenSCROLL,
    TokenSGN,
    TokenSIN,
    TokenSIZE,
    TokenSOURCE,
    TokenSPRITEA,
    TokenSPRITEC,
    TokenSPRITEX,
    TokenSPRITEY,
    TokenSPRITE,
    TokenSQR,
    TokenSTEP,
    TokenSTR,
    TokenSUB,
    TokenSWAP,
    TokenTAN,
    TokenTAP,
    TokenTEXT,
    TokenTHEN,
    TokenTIMER,
    TokenTO,
    TokenTOUCHX,
    TokenTOUCHY,
    TokenTOUCH,
    TokenTRACE,
    TokenUNTIL,
    TokenUP,
    TokenVAL,
    TokenVBL,
    TokenVOICEA,
    TokenVOICEF,
    TokenVOICEP,
    TokenVOICEV,
    TokenVOICE,
    TokenWAIT,
    TokenWEND,
    TokenWHILE,
    TokenWINDOW,
    
    // Reserved Keywords
    Token_reserved,
    TokenANIM,
    TokenBANK,
    TokenBAR,
    TokenBOX,
    TokenCANVAS,
    TokenCIRCLE,
    TokenCLOSE,
    TokenDECLARE,
    TokenDEF,
    TokenFLASH,
    TokenFN,
    TokenFUNCTION,
    TokenLBOUND,
    TokenLINE,
    TokenMUSIC,
    TokenOPEN,
    TokenOUTPUT,
    TokenPAINT,
    TokenPEN,
    TokenPLAY,
    TokenPLOT,
    TokenPOINT,
    TokenROL,
    TokenROR,
    TokenSHARED,
    TokenSOUND,
    TokenSTATIC,
    TokenSTOP,
    TokenTEMPO,
    TokenUBOUND,
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
