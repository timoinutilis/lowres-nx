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

#ifndef config_h
#define config_h

#if  defined(__EMSCRIPTEN__)
#define DEV_MENU 0
#define SCREENSHOTS 0
#define HOT_KEYS 0
#define SETTINGS_FILE 0

#elif defined(__LIBRETRO__)
#define DEV_MENU 1
#define SCREENSHOTS 0
#define HOT_KEYS 1
#define SETTINGS_FILE 1

#else
#define DEV_MENU 1
#define SCREENSHOTS 1
#define HOT_KEYS 1
#define SETTINGS_FILE 1
#endif

#endif /* config_h */
