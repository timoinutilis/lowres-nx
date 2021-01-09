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

bool data_canSetEntry(struct DataManager *manager, int index, int length);
void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length);

#endif /* data_manager_h */
