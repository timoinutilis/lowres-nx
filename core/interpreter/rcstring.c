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

#include "rcstring.h"
#include <stdlib.h>
#include <string.h>

struct RCString *rcstring_new(const char *chars, size_t len)
{
    size_t size = sizeof(struct RCString) + len;
    struct RCString *string = (struct RCString *) malloc(size);
    if (string)
    {
        string->refCount = 1; // retain
        if (chars)
        {
            memcpy(string->chars, chars, len);
        }
        string->chars[len] = 0; // end of string
    }
    return string;
}

void rcstring_retain(struct RCString *string)
{
    string->refCount++;
}

void rcstring_release(struct RCString *string)
{
    string->refCount--;
    if (string->refCount == 0)
    {
        free((void *)string);
    }
}
