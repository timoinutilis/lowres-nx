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

#if defined(_WIN32)
#include <windows.h>
#endif

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
#elif defined(__LINUX__)
    strncpy(buffer, getenv("HOME"), size - 1);
    strncat(buffer, "/Desktop/", size - 1);
#else
#error Not implemented yet
#endif
}

FILE* fopen_utf8(const char* filename, const char* mode)
{
#if defined(_WIN32)
	WCHAR nameW[FILENAME_MAX] = { 0 };
	WCHAR modeW[16] = { 0 };
	int len = MultiByteToWideChar(CP_UTF8, 0, filename, -1, nameW, FILENAME_MAX);
	if (len > 0 && MultiByteToWideChar(CP_UTF8, 0, mode, -1, modeW, 16) > 0)
	{
		FILE* ret = NULL;
		if (_wfopen_s(&ret, nameW, modeW) == 0)
			return ret;
	}
	return NULL;
#else
	return fopen(filename, mode);
#endif
}
