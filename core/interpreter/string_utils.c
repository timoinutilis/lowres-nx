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

#include "string_utils.h"
#include <stdlib.h>
#include <string.h>

const char *uppercaseString(const char *source)
{
    size_t len = strlen(source);
    char *buffer = (char *) malloc(len + 1);
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
        char *buffer = (char *) malloc(len + 1);
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
