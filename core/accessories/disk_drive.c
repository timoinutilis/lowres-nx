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
#include <stdint.h>

void disk_init(struct Core *core)
{
    // init lazily in disk_prepare()
}

void disk_deinit(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data)
    {
        free(dataManager->data);
        dataManager->data = NULL;
    }
    data_deinit(dataManager);
}

void disk_reset(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data)
    {
        data_reset(dataManager);
    }
}

bool disk_prepare(struct Core *core)
{
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (dataManager->data == NULL)
    {
        dataManager->data = calloc(DATA_SIZE, 1);
        if (!dataManager->data) exit(EXIT_FAILURE);
        
        data_init(dataManager);
    }
    return delegate_diskDriveWillAccess(core);
}

bool disk_saveFile(struct Core *core, int index, char *comment, int address, int length)
{
    if (!disk_prepare(core))
    {
        return false;
    }
    
    assert(address >= 0 && address < sizeof(struct Machine));
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    uint8_t *source = &((uint8_t *)core->machine)[address];
    data_setEntry(dataManager, index, comment, source, length);
    
    core->delegate->diskDriveDidSave(core->delegate->context, dataManager);
    return true;
}

bool disk_loadFile(struct Core *core, int index, int address)
{
    if (!disk_prepare(core))
    {
        return false;
    }
    
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
    return true;
}
