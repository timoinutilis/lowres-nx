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

#ifndef dev_mode_h
#define dev_mode_h

#include <stdio.h>
#include <stdbool.h>
#include "core.h"
#include "text_lib.h"

enum DevModeState {
    DevModeStateOff,
    DevModeStateVisible,
    DevModeStateRunningProgram,
    DevModeStateRunningTool
};

enum DevModeMenu {
    DevModeMenuMain,
    DevModeMenuTools
};

struct DevMode {
    enum DevModeState state;
    struct Core *core;
    bool lastTouch;
    enum DevModeMenu currentMenu;
    int currentButton;
    int currentMenuSize;
    char mainProgramFilename[FILENAME_MAX];
    struct CoreError lastError;
    struct TextLib textLib;
};

void dev_show(struct DevMode *devMode);
void dev_update(struct DevMode *devMode, struct CoreInput *input);

#endif /* dev_mode_h */
