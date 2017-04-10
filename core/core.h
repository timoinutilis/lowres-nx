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

#ifndef core_h
#define core_h

#include <stdio.h>
#include "machine.h"
#include "interpreter.h"

struct Core {
    struct Machine machine;
    struct Interpreter interpreter;
};

void core_init(struct Core *core);
void core_update(struct Core *core);
void core_rasterUpdate(struct Core *core);

#endif /* core_h */
