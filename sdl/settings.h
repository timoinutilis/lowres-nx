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

#ifndef settings_h
#define settings_h

#include <stdio.h>
#include <stdbool.h>

#define MAX_TOOLS 4
#define TOOL_NAME_SIZE 21

struct Parameters {
    bool fullscreen;
    bool fullwidth;
    bool disabledev;
    int mapping;
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
