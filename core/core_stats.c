//
// Copyright 2016-2020 Timo Kloss
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

#include "core_stats.h"
#include <string.h>
#include <stdlib.h>
#include "string_utils.h"

void stats_init(struct Stats *stats)
{
    memset(stats, 0, sizeof(struct Stats));
    
    stats->tokenizer = calloc(1, sizeof(struct Tokenizer));
    if (!stats->tokenizer) exit(EXIT_FAILURE);
    
    stats->romDataManager = calloc(1, sizeof(struct DataManager));
    if (!stats->romDataManager) exit(EXIT_FAILURE);
    
    stats->romDataManager->data = calloc(1, DATA_SIZE);
    if (!stats->romDataManager->data) exit(EXIT_FAILURE);
}

void stats_deinit(struct Stats *stats)
{
    tok_freeTokens(stats->tokenizer);
    
    free(stats->romDataManager->data);
    stats->romDataManager->data = NULL;
    
    free(stats->tokenizer);
    stats->tokenizer = NULL;
    
    free(stats->romDataManager);
    stats->romDataManager = NULL;
}

struct CoreError stats_update(struct Stats *stats, const char *sourceCode)
{
    stats->numTokens = 0;
    stats->romSize = 0;
    
    const char *upperCaseSourceCode = uppercaseString(sourceCode);
    if (!upperCaseSourceCode) return err_makeCoreError(ErrorOutOfMemory, -1);
    
    //TODO free resources on error
    
    struct CoreError error = tok_tokenizeUppercaseProgram(stats->tokenizer, upperCaseSourceCode);
    if (error.code != ErrorNone) return error;
    
    stats->numTokens = stats->tokenizer->numTokens;
    
    struct DataManager *romDataManager = stats->romDataManager;
    error = data_uppercaseImport(romDataManager, upperCaseSourceCode, false);
    if (error.code != ErrorNone) return error;
    
    stats->romSize = data_currentSize(stats->romDataManager);
    
    // add default characters if ROM entry 0 is unused
    struct DataEntry *entry0 = &romDataManager->entries[0];
    if (entry0->length == 0 && (DATA_SIZE - data_currentSize(romDataManager)) >= 1024)
    {
        stats->romSize += 1024;
    }
    
    tok_freeTokens(stats->tokenizer);
    free((void *)upperCaseSourceCode);
    
    return err_noCoreError();
}
