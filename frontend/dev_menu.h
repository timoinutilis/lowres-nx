//
// Copyright 2017 Timo Kloss
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
