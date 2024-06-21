//
// Copyright 2020 Timo Kloss
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

#include "core_stats.h"
#include <string.h>
#include <stdlib.h>
#include "string_utils.h"

void stats_init(struct Stats *stats)
{
    memset(stats, 0, sizeof(struct Stats));
    
    stats->tokenizer = (struct Tokenizer *) calloc(1, sizeof(struct Tokenizer));
    if (!stats->tokenizer) exit(EXIT_FAILURE);
    
    stats->romDataManager = (struct DataManager *) calloc(1, sizeof(struct DataManager));
    if (!stats->romDataManager) exit(EXIT_FAILURE);
    
    stats->romDataManager->data = (uint8_t *) calloc(1, DATA_SIZE);
    if (!stats->romDataManager->data) exit(EXIT_FAILURE);
}

void stats_deinit(struct Stats *stats)
{
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
    
    struct CoreError error = err_noCoreError();
    
    const char *upperCaseSourceCode = uppercaseString(sourceCode);
    if (!upperCaseSourceCode)
    {
        error = err_makeCoreError(ErrorOutOfMemory, -1);
        goto cleanup;
    }
    
    error = tok_tokenizeUppercaseProgram(stats->tokenizer, upperCaseSourceCode);
    if (error.code != ErrorNone)
    {
        goto cleanup;
    }
    
    stats->numTokens = stats->tokenizer->numTokens;
    
    struct DataManager *romDataManager = stats->romDataManager;
    error = data_uppercaseImport(romDataManager, upperCaseSourceCode, false);
    if (error.code != ErrorNone)
    {
        goto cleanup;
    }
    
    stats->romSize = data_currentSize(stats->romDataManager);
    
    // add default characters if ROM entry 0 is unused
    struct DataEntry *entry0 = &romDataManager->entries[0];
    if (entry0->length == 0 && (DATA_SIZE - data_currentSize(romDataManager)) >= 1024)
    {
        stats->romSize += 1024;
    }
    
cleanup:
    tok_freeTokens(stats->tokenizer);
    if (upperCaseSourceCode)
    {
        free((void *)upperCaseSourceCode);
    }
    
    return error;
}
