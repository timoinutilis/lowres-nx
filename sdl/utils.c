//
// Copyright 2018 Timo Kloss
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

#include "utils.h"
#include "system_paths.h"
#include <string.h>

void displayName(const char *filename, char *destination, size_t size)
{
    memset(destination, 0, size);
    
    const char *nameStart = filename;
    char *slash = strrchr(filename, PATH_SEPARATOR_CHAR);
    if (slash)
    {
        nameStart = slash + 1;
    }
    strncpy(destination, nameStart, size - 1);
    
    char *dot = strrchr(nameStart, '.');
    if (dot)
    {
        int dotIndex = (int)(dot - nameStart);
        if (dotIndex < size)
        {
            destination[dotIndex] = 0;
        }
    }
}

bool hasPostfix(const char *string, const char *postfix)
{
    size_t stringLen = strlen(string);
    size_t postfixLen = strlen(postfix);
    if (postfixLen <= stringLen)
    {
        string = string + stringLen - postfixLen;
        return strcmp(string, postfix) == 0;
    }
    return false;
}
