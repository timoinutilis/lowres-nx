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

#include "startup_sequence.h"
#include "core.h"
#include <string.h>

#define FONT_CHAR_OFFSET 192

void runStartupSequence(struct Core *core)
{
    struct DataEntry *entries = core->interpreter->romDataManager.entries;
    
    // init font and window
    core->interpreter->textLib.fontCharOffset = FONT_CHAR_OFFSET;
    txtlib_clearScreen(core);
    
    // default characters/font
    if (core->interpreter->romIncludesDefaultCharacters)
    {
        memcpy(&core->machine->videoRam.characters[FONT_CHAR_OFFSET], &core->machine->cartridgeRom[entries[0].start], entries[0].length);
    }
    
    // main palettes
    core->machine->colorRegisters.colors[1] = (3 << 4) | (3 << 2) | 3;
    core->machine->colorRegisters.colors[2] = (2 << 4) | (2 << 2) | 2;
    core->machine->colorRegisters.colors[3] = (1 << 4) | (1 << 2) | 1;
    memcpy(core->machine->colorRegisters.colors, &core->machine->cartridgeRom[entries[1].start], entries[1].length);
    
    // main characters
    memcpy(core->machine->videoRam.characters, &core->machine->cartridgeRom[entries[2].start], entries[2].length);

    // main background source
    core->interpreter->textLib.sourceAddress = entries[3].start;
    core->interpreter->textLib.sourceWidth = PLANE_COLUMNS;
}
