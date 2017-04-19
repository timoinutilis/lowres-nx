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

#include "token.h"

const char *TokenStrings[] = {
    NULL,
    
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    
    // Signs
    ":",
    ",",
    ";",
    NULL,
    
    // Operators
    "=",
    ">=",
    "<=",
    "<>",
    ">",
    "<",
    "(",
    ")",
    "+",
    "-",
    "*",
    "/",
    "^",
    "AND",
    "NOT",
    "OR",
    "XOR",
    
    // Commands
    "CLS",
    "COPY",
    "DATA",
    "DIM",
    "DO",
    "ELSE",
    "END",
    "FILL",
    "FOR",
    "GOSUB",
    "GOTO",
    "IF",
    "INPUT",
    "LET",
    "LOOP",
    "MOD",
    "NEXT",
    "NUMBER",
    "ON",
    "PEEK",
    "POKE",
    "PRINT",
    "RANDOMIZE",
    "RASTER",
    "READ",
    "REM",
    "REPEAT",
    "RESTORE",
    "RETURN",
    "STEP",
    "SWAP",
    "TEXT",
    "THEN",
    "TO",
    "UNTIL",
    "VBL",
    "WAIT",
    "WEND",
    "WHILE",
    
    // Functions
    "ABS",
    "ASC",
    "ATN",
    "BIN$",
    "CHR$",
    "COS",
    "EXP",
    "HEX$",
    "INKEY$",
    "INSTR",
    "INT",
    "LEFT$",
    "LENGTH",
    "LEN",
    "LOG",
    "MAX",
    "MID$",
    "MIN",
    "PI",
    "RIGHT$",
    "RND",
    "SGN",
    "SIN",
    "SQR",
    "START",
    "STR$",
    "TAN",
    "TIMER",
    "VAL",
    
    // Reserved Keywords
    NULL,
    "CALL",
    "DEF",
    "SHARED",
    "SUB",
    "UBOUND",
    
    NULL
};
