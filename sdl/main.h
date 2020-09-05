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

#ifndef main_h
#define main_h

#include <stdbool.h>

enum MainState {
    MainStateUndefined,
    MainStateBootIntro,
    MainStateRunningProgram,
    MainStateRunningTool,
    MainStateDevMenu,
};

enum Zoom {
    ZoomPixelPerfect,
    ZoomLarge,
    ZoomOverscan,
    ZoomSqueeze,
};

void bootNX(void);
void rebootNX(void);
bool hasProgram(void);
const char *getMainProgramFilename(void);
void selectProgram(const char *filename);
void runMainProgram(void);
void runToolProgram(const char *filename);
void showDevMenu(void);
bool usesMainProgramAsDisk(void);
void getDiskFilename(char *outputString);
void getRamFilename(char *outputString);
void setMouseEnabled(bool enabled);

#endif /* main_h */
