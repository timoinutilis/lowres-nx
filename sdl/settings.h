//
// Copyright 2017-2020 Timo Kloss
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

#ifndef settings_h
#define settings_h

#include <stdio.h>
#include <stdbool.h>

#define MAX_TOOLS 4
#define TOOL_NAME_SIZE 21

struct Parameters {
    bool fullscreen;
    int zoom;
    bool disabledev;
    int mapping;
    int disabledelay;
};

struct Settings {
    struct Parameters file;
    struct Parameters session;
    int numTools;
    char tools[MAX_TOOLS][FILENAME_MAX];
    char toolNames[MAX_TOOLS][TOOL_NAME_SIZE];
};

void settings_init(struct Settings *settings, char *filenameOut, int argc, const char * argv[]);
void settings_save(struct Settings *settings);
bool settings_addTool(struct Settings *settings, const char *filename);
void settings_removeTool(struct Settings *settings, int index);

#endif /* settings_h */
