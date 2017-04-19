//
// Copyright 2016 Timo Kloss
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

#include "core.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "text_lib.h"

void core_init(struct Core *core)
{
    machine_init(&core->machine);
    
    struct TextLib *textLib = &core->interpreter.textLib;
    textLib->charAttr.bank = 1;
    textLib->charAttr.priority = 1;
    textLib->charAttr.palette = 7;
    textLib->characterOffset = 192;
    textLib->areaX = 0;
    textLib->areaY = 0;
    textLib->areaWidth = 20;
    textLib->areaHeight = 16;
}

void core_update(struct Core *core)
{
    itp_runProgram(core);
}

void core_rasterUpdate(struct Core *core)
{
    itp_runRasterProgram(core);
}
