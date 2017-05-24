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

#include "disk_drive.h"
#include "core.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void disk_init(struct Core *core)
{
    core->diskDrive.data = calloc(DISK_SIZE, 1);
    assert(core->diskDrive.data != NULL);
}

void disk_deinit(struct Core *core)
{
    free(core->diskDrive.data);
    core->diskDrive.data = NULL;
}

void disk_saveFile(struct Core *core, char *name, int address, int length)
{
    int index;
    char comment[FILE_COMMENT_SIZE];
    int result = sscanf(name, "#%d:%s", &index, comment);
    if (result != 2)
    {
        assert(0);
    }
    else
    {
        struct FileEntry *entry = &core->diskDrive.entries[index];
        uint8_t *data = core->diskDrive.data;
        
        // move data of higher files
        int nextStart = entry->start + length;
        assert(nextStart < DISK_SIZE);
        
        if (length > entry->length) // new file is bigger
        {
            int diff = length - entry->length;
            for (int i = DISK_SIZE - 1; i >= nextStart; i--)
            {
                data[i] = data[i - diff];
            }
        }
        else if (length < entry->length) // new file is smaller
        {
            int diff = entry->length - length;
            for (int i = nextStart; i < DISK_SIZE - diff; i++)
            {
                data[i] = data[i + diff];
            }
            for (int i = DISK_SIZE - diff; i < DISK_SIZE; i++)
            {
                data[i] = 0;
            }
        }
        
        // write new file
        strncpy(entry->comment, comment, FILE_COMMENT_SIZE);
        entry->length = length;
        int start = entry->start;
        for (int i = 0; i < length; i++)
        {
            int peek = machine_peek(&core->machine, address + i);
            assert(peek != -1);
            data[i + start] = peek;
        }
        
        // move entry positions
        for (int i = index + 1; i < NUM_FILES; i++)
        {
            struct FileEntry *thisEntry = &core->diskDrive.entries[i];
            struct FileEntry *prevEntry = &core->diskDrive.entries[i - 1];
            thisEntry->start = prevEntry->start + prevEntry->length;
        }
    }
}

void disk_loadFile(struct Core *core, char *name, int address)
{
    int index;
    int result = sscanf(name, "#%d", &index);
    if (result != 1)
    {
        assert(0);
    }
    else
    {
        struct FileEntry *entry = &core->diskDrive.entries[index];
        uint8_t *data = core->diskDrive.data;
        
        // read file
        int start = entry->start;
        int length = entry->length;
        for (int i = 0; i < length; i++)
        {
            bool poke = machine_poke(&core->machine, address + i, data[i + start]);
            assert(poke);
        }
    }
}
