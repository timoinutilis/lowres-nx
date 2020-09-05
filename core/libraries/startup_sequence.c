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

#include "startup_sequence.h"
#include "core.h"
#include <string.h>
#include <stdint.h>

#define FONT_CHAR_OFFSET 192

void runStartupSequence(struct Core *core)
{
    struct DataEntry *entries = core->interpreter->romDataManager.entries;
    
    // init font and window
    struct TextLib *textLib = &core->interpreter->textLib;
    textLib->fontCharOffset = FONT_CHAR_OFFSET;
    txtlib_clearScreen(textLib);
    
    // default characters/font
    if (strcmp(entries[0].comment, "FONT") == 0)
    {
        memcpy(&core->machine->videoRam.characters[FONT_CHAR_OFFSET], &core->machine->cartridgeRom[entries[0].start], entries[0].length);
    }
    
    // default palettes
    uint8_t *colors = core->machine->colorRegisters.colors;
    
    colors[0] = (0 << 4) | (1 << 2) | 1;
    colors[1] = (3 << 4) | (3 << 2) | 3;
    colors[2] = (2 << 4) | (3 << 2) | 3;
    colors[3] = (0 << 4) | (0 << 2) | 0;

    colors[4] = 0;
    colors[5] = (3 << 4) | (2 << 2) | 0;
    colors[6] = (3 << 4) | (1 << 2) | 0;
    colors[7] = (0 << 4) | (0 << 2) | 0;
    
    colors[8] = 0;
    colors[9] = (3 << 4) | (3 << 2) | 0;
    colors[10] = (0 << 4) | (3 << 2) | 0;
    colors[11] = (0 << 4) | (0 << 2) | 0;

    colors[12] = 0;
    colors[13] = (3 << 4) | (3 << 2) | 3;
    colors[14] = (3 << 4) | (3 << 2) | 0;
    colors[15] = (0 << 4) | (0 << 2) | 0;
    
    for (int i = 0; i < 16; i += 4)
    {
        colors[16 + i] = 0;
        colors[17 + i] = (3 << 4) | (3 << 2) | 3;
        colors[18 + i] = (2 << 4) | (2 << 2) | 2;
        colors[19 + i] = (1 << 4) | (1 << 2) | 1;
    }
    
    // main palettes
    int palLen = entries[1].length;
    if (palLen > 32) palLen = 32;
    memcpy(core->machine->colorRegisters.colors, &core->machine->cartridgeRom[entries[1].start], palLen);
    
    // main characters
    memcpy(core->machine->videoRam.characters, &core->machine->cartridgeRom[entries[2].start], entries[2].length);

    // main background source
    int bgStart = entries[3].start;
    core->interpreter->textLib.sourceAddress = bgStart + 4;
    core->interpreter->textLib.sourceWidth = core->machine->cartridgeRom[bgStart + 2];
    core->interpreter->textLib.sourceHeight = core->machine->cartridgeRom[bgStart + 3];
    
    // voices
    for (int i = 0; i < NUM_VOICES; i++)
    {
        struct Voice *voice = &core->machine->audioRegisters.voices[i];
        voice->attr.pulseWidth = 8;
        voice->status.volume = 15;
        voice->status.mix = 3;
        voice->envS = 15;
    }
    
    // main sound source
    core->interpreter->audioLib.sourceAddress = entries[15].start;
}
