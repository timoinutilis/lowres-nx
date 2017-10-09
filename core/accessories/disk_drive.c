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
}

void disk_deinit(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data)
    {
        free(dataManager->data);
        dataManager->data = NULL;
    }
}

void disk_prepare(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data == NULL)
    {
        dataManager->data = calloc(DATA_SIZE, 1);
        assert(dataManager->data != NULL);
        
        core->delegate->diskDriveWillAccess(core->delegate->context, dataManager);
    }
}

void disk_saveFile(struct Core *core, char *name, int address, int length)
{
    disk_prepare(core);
    
    int index;
    char comment[ENTRY_COMMENT_SIZE];
    int result = sscanf(name, "#%d:%[^\n]", &index, comment);
    if (result != 2)
    {
        assert(0);
    }
    else
    {
        assert(address >= 0 && address < sizeof(struct Machine));
        struct DataManager *dataManager = &core->diskDrive->dataManager;
        uint8_t *source = &((uint8_t *)core->machine)[address];
        data_setEntry(dataManager, index, comment, source, length);
        
        core->delegate->diskDriveDidSave(core->delegate->context, dataManager);
    }
}

void disk_loadFile(struct Core *core, char *name, int address)
{
    disk_prepare(core);
    
    int index;
    int result = sscanf(name, "#%d", &index);
    if (result != 1)
    {
        assert(0);
    }
    else
    {
        struct DataEntry *entry = &core->diskDrive->dataManager.entries[index];
        uint8_t *data = core->diskDrive->dataManager.data;
        
        // read file
        int start = entry->start;
        int length = entry->length;
        for (int i = 0; i < length; i++)
        {
            bool poke = machine_poke(core, address + i, data[i + start]);
            assert(poke);
        }
    }
}
