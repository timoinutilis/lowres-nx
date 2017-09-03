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
    if (core->diskDrive.data)
    {
        free(core->diskDrive.data);
        core->diskDrive.data = NULL;
    }
}

void disk_prepare(struct Core *core)
{
    if (core->diskDrive.data == NULL)
    {
        core->diskDrive.data = calloc(DISK_SIZE, 1);
        assert(core->diskDrive.data != NULL);
        
        core->delegate->diskDriveWillAccess(core->delegate->context);
    }
}

bool disk_importDisk(struct Core *core, const char *input)
{
    const char *character = input;
    uint8_t *currentDataByte = core->diskDrive.data;
    uint8_t *endDataByte = &core->diskDrive.data[DISK_SIZE];
    
    // skip stuff before
    //TODO: make sure # is start of line
    while (*character && *character != '#')
    {
        character++;
    }
    
    while (*character)
    {
        if (*character == '#')
        {
            character++;
            
            // entry index
            int entryIndex = 0;
            while (*character)
            {
                if (strchr(CharSetDigits, *character))
                {
                    int digit = (int)*character - (int)'0';
                    entryIndex *= 10;
                    entryIndex += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            if (*character != ':') return false;
            character++;
            
            if (entryIndex >= NUM_FILES) return false;
            
            struct FileEntry *entry = &core->diskDrive.entries[entryIndex];
            if (entry->length > 0) return false;
            
            // file comment
            const char *comment = character;
            do
            {
                character++;
            }
            while (*character && *character != '\n');
            size_t commentLen = (character - comment);
            if (commentLen > FILE_COMMENT_SIZE) commentLen = FILE_COMMENT_SIZE - 1;
            strncpy(entry->comment, comment, commentLen);
            
            // binary data
            uint8_t *startByte = currentDataByte;
            bool shift = true;
            int value = 0;
            while (*character && *character != '#')
            {
                char *spos = strchr(CharSetHex, *character);
                if (spos)
                {
                    int digit = (int)(spos - CharSetHex);
                    if (shift)
                    {
                        value = digit << 4;
                    }
                    else
                    {
                        value |= digit;
                        if (currentDataByte >= endDataByte) return ErrorRomIsFull;
                        *currentDataByte = value;
                        ++currentDataByte;
                    }
                    shift = !shift;
                }
                else if (*character != ' ' && *character == '\t' && *character == '\n')
                {
                    return false;
                }
                character++;
            }
            if (!shift) return false; // incomplete hex value
            
            int start = (int)(startByte - core->diskDrive.data);
            int length = (int)(currentDataByte - startByte);
            entry->start = start;
            entry->length = length;
            
            for (int i = entryIndex + 1; i < NUM_FILES; i++)
            {
                core->diskDrive.entries[i].start = entry->start + entry->length;
            }
        }
        else if (*character == ' ' || *character == '\t' || *character == '\n')
        {
            character++;
        }
        else
        {
            return false;
        }
    }
    return true;
}

int disk_calcOutputSize(struct DiskDrive *diskDrive)
{
    int size = 0;
    for (int i = 0; i < NUM_FILES; i++)
    {
        struct FileEntry *entry = &diskDrive->entries[i];
        if (entry->length > 0)
        {
            size += (i >= 10 ? 4 : 3) + strlen(entry->comment) + 1; // #10:comment\n
            size += entry->length * 3; // 2x hex letters + space or new line
            size += 1; // new line
        }
    }
    return size;
}

char *disk_exportDisk(struct Core *core)
{
    size_t outputSize = disk_calcOutputSize(&core->diskDrive);
    if (outputSize > 0)
    {
        char *output = calloc(outputSize, 1);
        char *current = output;
        if (output)
        {
            for (int i = 0; i < NUM_FILES; i++)
            {
                struct FileEntry *entry = &core->diskDrive.entries[i];
                if (entry->length > 0)
                {
                    sprintf(current, "#%d:%s\n", i, entry->comment);
                    current += strlen(current);
                    int valuesInLine = 0;
                    int pos = 0;
                    uint8_t *entryData = &core->diskDrive.data[entry->start];
                    while (pos < entry->length)
                    {
                        sprintf(current, "%02X", entryData[pos]);
                        current += strlen(current);
                        pos++;
                        valuesInLine++;
                        if (pos == entry->length)
                        {
                            sprintf(current, "\n\n");
                        }
                        else if (valuesInLine == 16)
                        {
                            sprintf(current, "\n");
                            valuesInLine = 0;
                        }
                        else
                        {
                            sprintf(current, " ");
                        }
                        current += strlen(current);
                    }
                    
                }
            }
            
            core->diskDrive.hasChanges = false;
        }
        return output;
    }
    return NULL;
}

void disk_saveFile(struct Core *core, char *name, int address, int length)
{
    disk_prepare(core);
    
    int index;
    char comment[FILE_COMMENT_SIZE];
    int result = sscanf(name, "#%d:%[^\n]", &index, comment);
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
            int peek = machine_peek(core, address + i);
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
        
        core->diskDrive.hasChanges = true;
        core->delegate->diskDriveDidSave(core->delegate->context);
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
        struct FileEntry *entry = &core->diskDrive.entries[index];
        uint8_t *data = core->diskDrive.data;
        
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
