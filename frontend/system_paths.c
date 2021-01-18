//
// Copyright 2018 Timo Kloss
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
#elif defined(__LIBRETRO__)
	// not used
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
