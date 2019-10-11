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
    
    "Could Not Open Program",
    "Too Many Tokens",
    "ROM Is Full",
    "Index Already Defined",
    "Unterminated String",
    "Unexpected Character",
    "Reserved Keyword",
    "Syntax Error",
    "Symbol Name Too Long",
    "Too Many Symbols",
    "Type Mismatch",
    "Out Of Memory",
    "ELSE Without IF",
    "END IF Without IF",
    "Expected Command",
    "NEXT Without FOR",
    "LOOP Without DO",
    "UNTIL Without REPEAT",
    "WEND Without WHILE",
    "Label Already Defined",
    "Too Many Labels",
    "ErrorExpectedLabel",
    "Undefined Label",
    "Array Not Dimensionized",
    "Array Already Dimensionized",
    "Variable Already Used",
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
    "Array Variable Without Index",
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
    "Keyboard Not Enabled",
    "Automatic Pause Not Disabled",
    "Gamepad Not Enabled",
    "Touch Not Enabled",
    "Input Change Not Allowed",
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
