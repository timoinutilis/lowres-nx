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

#ifndef data_manager_h
#define data_manager_h

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "error.h"

#define MAX_ENTRIES 16
#define DATA_SIZE 0x8000
#define ENTRY_COMMENT_SIZE 32

struct DataEntry {
    char comment[ENTRY_COMMENT_SIZE];
    int start;
    int length;
};

struct DataManager {
    struct DataEntry entries[MAX_ENTRIES];
    uint8_t *data;
    const char *diskSourceCode;
};

void data_init(struct DataManager *manager);
void data_deinit(struct DataManager *manager);
void data_reset(struct DataManager *manager);
struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode);
struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode);
char *data_export(struct DataManager *manager);

int data_currentSize(struct DataManager *manager);

void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length);

#endif /* data_manager_h */
