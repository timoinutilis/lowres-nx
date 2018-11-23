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

#include "data_manager.h"
#include "charsets.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "string_utils.h"

int data_calcOutputSize(struct DataManager *manager);

void data_init(struct DataManager *manager)
{
    data_reset(manager);
}

void data_deinit(struct DataManager *manager)
{
    assert(manager);
    
    if (manager->diskSourceCode)
    {
        free((void *)manager->diskSourceCode);
        manager->diskSourceCode = NULL;
    }
}

void data_reset(struct DataManager *manager)
{
    memset(manager->entries, 0, sizeof(struct DataEntry) * MAX_ENTRIES);
    
    strcpy(manager->entries[1].comment, "MAIN PALETTES");
    strcpy(manager->entries[2].comment, "MAIN CHARACTERS");
    strcpy(manager->entries[3].comment, "MAIN BG");
    strcpy(manager->entries[15].comment, "MAIN SOUND");

    if (manager->diskSourceCode)
    {
        free((void *)manager->diskSourceCode);
        manager->diskSourceCode = NULL;
    }
}

struct CoreError data_import(struct DataManager *manager, const char *input, bool keepSourceCode)
{
    assert(manager);
    assert(input);
    
    const char *uppercaseInput = uppercaseString(input);
    if (!uppercaseInput) return err_makeCoreError(ErrorOutOfMemory, -1);
    
    struct CoreError error = data_uppercaseImport(manager, uppercaseInput, keepSourceCode);
    free((void *)uppercaseInput);
    
    return error;
}

struct CoreError data_uppercaseImport(struct DataManager *manager, const char *input, bool keepSourceCode)
{
    assert(manager);
    assert(input);
    
    data_reset(manager);
    
    const char *character = input;
    uint8_t *currentDataByte = manager->data;
    uint8_t *endDataByte = &manager->data[DATA_SIZE];
    
    // skip stuff before
    const char *prevChar = NULL;
    while (*character && !(*character == '#' && (!prevChar || *prevChar == '\n')))
    {
        prevChar = character;
        character++;
    }
    
    if (keepSourceCode)
    {
        size_t length = (size_t)(character - input);
        
        char *diskSourceCode = malloc(length + 1);
        if (!diskSourceCode) exit(EXIT_FAILURE);
        
        stringConvertCopy(diskSourceCode, input, length);
        manager->diskSourceCode = diskSourceCode;
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
            if (*character != ':') return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - input));
            character++;
            
            if (entryIndex >= MAX_ENTRIES) return err_makeCoreError(ErrorIndexOutOfBounds, (int)(character - input));
            
            struct DataEntry *entry = &manager->entries[entryIndex];
            if (entry->length > 0) return err_makeCoreError(ErrorIndexAlreadyDefined, (int)(character - input));
            
            // file comment
            const char *comment = character;
            do
            {
                character++;
            }
            while (*character && *character != '\n' && *character != '\r');
            size_t commentLen = (character - comment);
            if (commentLen >= ENTRY_COMMENT_SIZE) commentLen = ENTRY_COMMENT_SIZE - 1;
            memset(entry->comment, 0, ENTRY_COMMENT_SIZE);
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
                        if (currentDataByte >= endDataByte) return err_makeCoreError(ErrorRomIsFull, (int)(character - input));
                        *currentDataByte = value;
                        ++currentDataByte;
                    }
                    shift = !shift;
                }
                else if (*character != ' ' && *character != '\t' && *character != '\n' && *character != '\r')
                {
                    return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - input));
                }
                character++;
            }
            if (!shift) return err_makeCoreError(ErrorSyntax, (int)(character - input)); // incomplete hex value
            
            int start = (int)(startByte - manager->data);
            int length = (int)(currentDataByte - startByte);
            entry->start = start;
            entry->length = length;
            
            for (int i = entryIndex + 1; i < MAX_ENTRIES; i++)
            {
                manager->entries[i].start = entry->start + entry->length;
            }
        }
        else if (*character == ' ' || *character == '\t' || *character == '\n' || *character == '\r')
        {
            character++;
        }
        else
        {
            return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - input));
        }
    }
    return err_noCoreError();
}

char *data_export(struct DataManager *manager)
{
    assert(manager);
    
    size_t outputSize = data_calcOutputSize(manager);
    if (outputSize > 0)
    {
        char *output = malloc(outputSize);
        if (output)
        {
            char *current = output;
            
            if (manager->diskSourceCode)
            {
                size_t len = strlen(manager->diskSourceCode);
                if (len > 0)
                {
                    strcpy(current, manager->diskSourceCode);
                    char endChar = current[len - 1];
                    current += len;
                    if (endChar != '\n')
                    {
                        // add new line after end of program
                        current[0] = '\n';
                        current++;
                    }
                }
            }
            
            for (int i = 0; i < MAX_ENTRIES; i++)
            {
                struct DataEntry *entry = &manager->entries[i];
                if (entry->length > 0)
                {
                    sprintf(current, "#%d:%s\n", i, entry->comment);
                    current += strlen(current);
                    int valuesInLine = 0;
                    int pos = 0;
                    uint8_t *entryData = &manager->data[entry->start];
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
                        current += strlen(current);
                    }
                    
                }
            }
        }
        return output;
    }
    return NULL;
}

int data_calcOutputSize(struct DataManager *manager)
{
    int size = 0;
    for (int i = 0; i < MAX_ENTRIES; i++)
    {
        struct DataEntry *entry = &manager->entries[i];
        if (entry->length > 0)
        {
            size += (i >= 10 ? 4 : 3) + strlen(entry->comment) + 1; // #10:comment\n
            size += entry->length * 2; // 2x hex letters
            size += entry->length / 16 + 1; // new line every 16 values
            size += 1; // new line
        }
    }
    if (manager->diskSourceCode)
    {
        size += strlen(manager->diskSourceCode) + 1; // possible new line between program and data
    }
    size += 1; // 0-byte
    return size;
}

int data_currentSize(struct DataManager *manager)
{
    int size = 0;
    for (int i = 0; i < MAX_ENTRIES; i++)
    {
        size += manager->entries[i].length;
    }
    return size;
}

void data_setEntry(struct DataManager *manager, int index, const char *comment, uint8_t *source, int length)
{
    struct DataEntry *entry = &manager->entries[index];
    uint8_t *data = manager->data;
        
    // move data of higher entries
    int nextStart = entry->start + length;
    assert(nextStart < DATA_SIZE);
        
    if (length > entry->length) // new entry is bigger
    {
        int diff = length - entry->length;
        for (int i = DATA_SIZE - 1; i >= nextStart; i--)
        {
            data[i] = data[i - diff];
        }
    }
    else if (length < entry->length) // new entry is smaller
    {
        int diff = entry->length - length;
        for (int i = nextStart; i < DATA_SIZE - diff; i++)
        {
            data[i] = data[i + diff];
        }
        for (int i = DATA_SIZE - diff; i < DATA_SIZE; i++)
        {
            data[i] = 0;
        }
    }
    
    // write new entry
    strncpy(entry->comment, comment, ENTRY_COMMENT_SIZE);
    entry->comment[ENTRY_COMMENT_SIZE - 1] = 0;
    entry->length = length;
    int start = entry->start;
    for (int i = 0; i < length; i++)
    {
        data[i + start] = source[i];
    }
    
    // move entry positions
    for (int i = index + 1; i < MAX_ENTRIES; i++)
    {
        struct DataEntry *thisEntry = &manager->entries[i];
        struct DataEntry *prevEntry = &manager->entries[i - 1];
        thisEntry->start = prevEntry->start + prevEntry->length;
    }
}
