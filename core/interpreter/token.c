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
    
    // Commands/Functions
    "ABS",
    "ASC",
    "ATN",
    "ATRB",
    "BG",
    "BIN$",
    "BUTTON",
    "CELL",
    "CHAR",
    "CHR$",
    "CLS",
    "CLW",
    "COLOR",
    "COPY",
    "COS",
    "DATA",
    "DIM",
    "DISPLAY",
    "DOWN",
    "DO",
    "ELSE",
    "END",
    "EXP",
    "FILL",
    "FONT",
    "FOR",
    "GOSUB",
    "GOTO",
    "HEX$",
    "IF",
    "INKEY$",
    "INPUT",
    "INSTR",
    "INT",
    "KEY",
    "LEFT$",
    "LEFT",
    "LENGTH",
    "LEN",
    "LET",
    "LOCATE",
    "LOG",
    "LOOP",
    "MAX",
    "MID$",
    "MIN",
    "MOD",
    "MOUSE",
    "NEXT",
    "NUMBER",
    "ON",
    "PALETTE",
    "PEEK",
    "PI",
    "POKE",
    "PRINT",
    "RANDOMIZE",
    "RASTER",
    "READ",
    "REM",
    "REPEAT",
    "RESTORE",
    "RETURN",
    "RIGHT$",
    "RIGHT",
    "RND",
    "SCROLL",
    "SGN",
    "SIN",
    "SIZE",
    "SOURCE",
    "SPRITE",
    "SQR",
    "START",
    "STEP",
    "STR$",
    "SWAP",
    "TAN",
    "TAP",
    "TEXT",
    "THEN",
    "TIMER",
    "TO",
    "UNTIL",
    "UP",
    "VAL",
    "VBL",
    "WAIT",
    "WEND",
    "WHILE",
    "WINDOW",
    
    // Reserved Keywords
    NULL,
    "CALL",
    "DEF",
    "SHARED",
    "SUB",
    "UBOUND",
    
    NULL
};
