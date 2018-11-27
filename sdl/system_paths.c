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

#include "system_paths.h"
#include <stdlib.h>
#include <string.h>

void desktop_path(char *buffer, size_t size)
{
#if defined(__APPLE__) && defined(__MACH__)
    strncpy(buffer, getenv("HOME"), size - 1);
    strncat(buffer, "/Desktop/", size - 1);
#elif defined(_WIN32)
	strncpy(buffer, getenv("USERPROFILE"), size - 1);
	strncat(buffer, "\\Desktop\\", size - 1);
#elif defined(__EMSCRIPTEN__)
    strncpy(buffer, "", size - 1);
#else
//error Not implemented yet
#endif
}
