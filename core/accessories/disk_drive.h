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
bool disk_loadFile(struct Core *core, int index, int address);

#endif /* disk_drive_h */
