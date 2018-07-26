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

#ifndef string_utils_h
#define string_utils_h

#include <stdio.h>

const char *uppercaseString(const char *source);
const char *lineString(const char *source, int pos);
int lineNumber(const char *source, int pos);
void stringConvertCopy(char *dest, const char *source, size_t length);

#endif /* string_utils_h */
