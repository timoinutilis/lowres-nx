//
// Copyright 2017-2020 Timo Kloss
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
    TokenCURSORX,
    TokenCURSORY,
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
    TokenUBOUND,
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
