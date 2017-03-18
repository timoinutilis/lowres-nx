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

#include "rcstring.h"
#include <stdlib.h>
#include <string.h>

struct RCString *rcstring_new(const char *chars, size_t len)
{
    size_t size = sizeof(struct RCString) + len;
    struct RCString *string = malloc(size);
    if (string)
    {
        string->refCount = 1; // retain
        if (chars)
        {
            memcpy(string->chars, chars, len);
        }
        string->chars[len] = 0; // end of string
        printf("new string %lx: %s\n", (unsigned long)string, string->chars);
    }
    return string;
}

void rcstring_retain(struct RCString *string)
{
    string->refCount++;
//    printf("retain string %lx refc: %d\n", (unsigned long)string, string->refCount);
}

void rcstring_release(struct RCString *string)
{
    string->refCount--;
//    printf("release string %lx refc: %d\n", (unsigned long)string, string->refCount);
    if (string->refCount == 0)
    {
        printf("--- free\n");
        free((void *)string);
    }
}
