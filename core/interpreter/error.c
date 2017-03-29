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

#include "error.h"

const char *ErrorStrings[] = {
    "OK",
    "Too Many Tokens",
    "ROM is full",
    "Expected End Of String",
    "Unexpected Character",
    "Syntax Error",
    "End Of Program",
    "Unexpected Token",
    "Expected End Of Line",
    "Expected THEN",
    "Expected Equal Sign '='",
    "Expected Variable Identifier",
    "Expected Left Parenthesis '('",
    "Expected Right Parenthesis ')'",
    "Symbol Name Too Long",
    "Too Many Symbols",
    "Type Mismatch",
    "Out Of Memory",
    "ELSE Without IF",
    "END IF Without IF",
    "Expected Command",
    "Expected TO",
    "NEXT Without FOR",
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
    "Out Of Data"
};
