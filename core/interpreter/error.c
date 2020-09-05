//
// Copyright 2017 Timo Kloss
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
    "EXIT Not Inside Loop",
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
