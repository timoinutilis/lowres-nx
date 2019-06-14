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

#ifndef dev_menu_h
#define dev_menu_h

#include "config.h"

#if DEV_MENU

#include <stdio.h>
#include <stdbool.h>
#include "core.h"
#include "settings.h"
#include "runner.h"
#include "text_lib.h"

enum DevModeMenu {
    DevModeMenuMain,
    DevModeMenuTools,
    DevModeMenuClearRam
};

struct DevMenu {
    struct Runner *runner;
    struct Settings *settings;
    bool lastTouch;
    enum DevModeMenu currentMenu;
    int currentButton;
    int currentMenuSize;
    struct CoreError lastError;
    struct TextLib textLib;
};

void dev_init(struct DevMenu *devMenu, struct Runner *runner, struct Settings *settings);
void dev_show(struct DevMenu *devMenu, bool reload);
void dev_update(struct DevMenu *devMenu, struct CoreInput *input);
bool dev_handleDropFile(struct DevMenu *devMenu, const char *filename);

#endif

#endif /* dev_menu_h */
