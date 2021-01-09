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

#ifndef disk_drive_h
#define disk_drive_h

#include <stdio.h>
#include <stdbool.h>
#include "data_manager.h"

struct Core;

struct DiskDrive {
    struct DataManager dataManager;
};

void disk_init(struct Core *core);
void disk_deinit(struct Core *core);
void disk_reset(struct Core *core);

bool disk_prepare(struct Core *core);
bool disk_saveFile(struct Core *core, int index, char *comment, int address, int length);
bool disk_loadFile(struct Core *core, int index, int address, int maxLength, int offset, bool *pokeFailed);

#endif /* disk_drive_h */
