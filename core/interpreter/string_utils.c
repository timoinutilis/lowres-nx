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

#include "string_utils.h"
#include <stdlib.h>
#include <string.h>

const char *uppercaseString(const char *source)
{
    size_t len = strlen(source);
    char *buffer = malloc(len + 1);
    if (buffer)
    {
        const char *sourceChar = source;
        char *destChar = buffer;
        char finalChar = 0;
        while (*sourceChar)
        {
            finalChar = *sourceChar++;
            if (finalChar >= 'a' && finalChar <= 'z')
            {
                finalChar -= 32;
            }
            *destChar++ = finalChar;
        }
        *destChar = 0;
    }
    return buffer;
}

const char *lineString(const char *source, int pos)
{
    const char *start = &source[pos];
    const char *end = &source[pos];
    while (start - 1 >= source && *(start - 1) != '\n')
    {
        start--;
    }
    while (*(end + 1) != 0 && *end != '\n' && *end != 0)
    {
        end++;
    }
    if (end > start)
    {
        size_t len = end - start;
        char *buffer = malloc(len + 1);
        if (buffer)
        {
            strncpy(buffer, start, len);
            buffer[len] = 0;
            return buffer;
        }
    }
    return NULL;
}

int lineNumber(const char *source, int pos)
{
    int line = 1;
    for (int i = 0; i < pos; i++)
    {
        if (source[i] == '\n')
        {
            line++;
        }
    }
    return line;
}

void stringConvertCopy(char *dest, const char *source, size_t length)
{
    char *currDstChar = dest;
    for (int i = 0; i < length; i++)
    {
        char currSrcChar = source[i];
        if (currSrcChar != '\r')
        {
            *currDstChar = currSrcChar;
            currDstChar++;
        }
    }
    *currDstChar = 0;
}
