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

#define NUM_FILES 16
#define DISK_SIZE 32768
#define FILE_COMMENT_SIZE 32

struct Core;

struct FileEntry {
    char comment[FILE_COMMENT_SIZE];
    int start;
    int length;
};

struct DiskDrive {
    struct FileEntry entries[NUM_FILES];
    uint8_t *data;
    bool hasChanges;
};

void disk_init(struct Core *core);
void disk_deinit(struct Core *core);
bool disk_importDisk(struct Core *core, const char *input);
char *disk_exportDisk(struct Core *core);

void disk_saveFile(struct Core *core, char *name, int address, int length);
void disk_loadFile(struct Core *core, char *name, int address);

#endif /* disk_drive_h */
