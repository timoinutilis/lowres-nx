//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#include "lowres_core.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "text_lib.h"

void LRC_init(struct LowResCore *core)
{
    LRC_initMachine(&core->machine);
    
    struct TextLib *textLib = &core->interpreter.textLib;
    textLib->charAttr.bank = 1;
    textLib->charAttr.priority = 1;
    textLib->charAttr.palette = 7;
    textLib->characterOffset = 128;
    textLib->areaX = 0;
    textLib->areaY = 0;
    textLib->areaWidth = 16;
    textLib->areaHeight = 16;
}

void LRC_update(struct LowResCore *core)
{
    LRC_runProgram(core);
}

void LRC_rasterUpdate(struct LowResCore *core)
{
    if (core->interpreter.currentOnRasterToken)
    {
        struct Token *pc = core->interpreter.pc;
        core->interpreter.pc = core->interpreter.currentOnRasterToken;
        LRC_runProgram(core);
        core->interpreter.pc = pc;
    }
}
