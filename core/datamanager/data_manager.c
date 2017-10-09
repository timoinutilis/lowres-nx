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


enum ErrorCode data_import(struct DataManager *manager, const char *input)
{
    const char *uppercaseInput = uppercaseString(input);
    if (!uppercaseInput) return ErrorOutOfMemory;
    
    enum ErrorCode errorCode = data_uppercaseImport(manager, uppercaseInput);
    free((void *)uppercaseInput);
    
    return errorCode;
}

enum ErrorCode data_uppercaseImport(struct DataManager *manager, const char *input)
{
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
            if (*character != ':') return ErrorUnexpectedCharacter;
            character++;
            
            if (entryIndex >= MAX_ENTRIES) return ErrorIndexOutOfBounds;
            
            struct DataEntry *entry = &manager->entries[entryIndex];
            if (entry->length > 0) return ErrorIndexAlreadyDefined;
            
            // file comment
            const char *comment = character;
            do
            {
                character++;
            }
            while (*character && *character != '\n');
            size_t commentLen = (character - comment);
            if (commentLen > ENTRY_COMMENT_SIZE) commentLen = ENTRY_COMMENT_SIZE - 1;
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
                    return ErrorUnexpectedCharacter;
                }
                character++;
            }
            if (!shift) return ErrorSyntax; // incomplete hex value
            
            int start = (int)(startByte - manager->data);
            int length = (int)(currentDataByte - startByte);
            entry->start = start;
            entry->length = length;
            
            for (int i = entryIndex + 1; i < MAX_ENTRIES; i++)
            {
                manager->entries[i].start = entry->start + entry->length;
            }
        }
        else if (*character == ' ' || *character == '\t' || *character == '\n')
        {
            character++;
        }
        else
        {
            return ErrorUnexpectedCharacter;
        }
    }
    return ErrorNone;
}

char *data_export(struct DataManager *manager)
{
    size_t outputSize = data_calcOutputSize(manager);
    if (outputSize > 0)
    {
        char *output = calloc(outputSize, 1);
        char *current = output;
        if (output)
        {
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
                        else
                        {
                            sprintf(current, " ");
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
            size += entry->length * 3; // 2x hex letters + space or new line
            size += 1; // new line
        }
    }
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
