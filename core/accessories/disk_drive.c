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
    
    assert(address >= 0 && address + length <= sizeof(struct Machine));
    struct DataManager *dataManager = &core->diskDrive->dataManager;
    if (!data_canSetEntry(dataManager, index, length))
    {
        delegate_diskDriveIsFull(core);
    }
    else
    {
        uint8_t *source = &((uint8_t *)core->machine)[address];
        data_setEntry(dataManager, index, comment, source, length);
        
        delegate_diskDriveDidSave(core);
    }
    return true;
}

bool disk_loadFile(struct Core *core, int index, int address, int maxLength, int offset, bool *pokeFailed)
{
    if (!disk_prepare(core))
    {
        return false;
    }
    
    struct DataEntry *entry = &core->diskDrive->dataManager.entries[index];
    uint8_t *data = core->diskDrive->dataManager.data;
    
    // read file
    int start = entry->start + offset;
    int length = entry->length;
    if (maxLength > 0 && length > maxLength)
    {
        length = maxLength;
    }
    if (offset + length > entry->length)
    {
        length = entry->length - offset;
    }
    for (int i = 0; i < length; i++)
    {
        bool poke = machine_poke(core, address + i, data[i + start]);
        if (!poke)
        {
            *pokeFailed = true;
            return true;
        }
    }
    return true;
}
