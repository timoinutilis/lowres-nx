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

#include "error.h"

const char *ErrorStrings[] = {
    "OK",
    
    "Too Many Tokens",
    "ROM is full",
    "Index Already Defined",
    "Unterminated String",
    "Unexpected Character",
    "Reserved Keyword",
    "Syntax Error",
    "Unexpected Token",
    "Expected End Of Line",
    "Expected THEN",
    "Expected Equal Sign '='",
    "Expected Variable Identifier",
    "Expected Left Parenthesis '('",
    "Expected Right Parenthesis ')'",
    "Expected Comma ','",
    "Expected Semicolon ';'",
    "Symbol Name Too Long",
    "Too Many Symbols",
    "Type Mismatch",
    "Out Of Memory",
    "ELSE Without IF",
    "END IF Without IF",
    "Expected Command",
    "Expected TO",
    "NEXT Without FOR",
    "LOOP Without DO",
    "UNTIL Without REPEAT",
    "WEND Without WHILE",
    "Label Already Defined",
    "Too Many Labels",
    "ErrorExpectedLabel",
    "Undefined Label",
    "Expected Numeric Expression",
    "Expected String Expression",
    "Array Not Dimensionized",
    "Array Already Dimensionized",
    "Index Out Of Bounds",
    "Wrong Number Of Dimensions",
    "Invalid Parameter",
    "RETURN Without GOSUB",
    "Stack Overflow",
    "Out Of Data",
    "Illegal Memory Access",
    "Too Many CPU Cycles In Interrupt",
    "Not Allowed In Interrupt",
    "IF Without END IF",
    "FOR Without NEXT",
    "DO Without LOOP",
    "REPEAT Without UNTIL",
    "WHILE Without WEND",
    "Directory Not Loaded",
    "Division By Zero",
    "Variable Not Initialized",
    "END SUB Without SUB",
    "SUB Without END SUB",
    "SUB Cannot Be Nested",
    "Undefined Subprogram",
    "Expected Subprogram Name",
    "Argument Count Mismatch",
    "SUB Already Defined",
    "Too Many Subprograms",
    "SHARED Outside Of A Subprogram",
    "GLOBAL Inside Of A Subprogram",
    "EXIT SUB Outside Of A Subprogram",
};

const char *err_getString(enum ErrorCode errorCode)
{
    return ErrorStrings[errorCode];
}

struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition)
{
    struct CoreError error = {code, sourcePosition};
    return error;
}

struct CoreError err_noCoreError(void)
{
    struct CoreError error = {ErrorNone, 0};
    return error;
}
