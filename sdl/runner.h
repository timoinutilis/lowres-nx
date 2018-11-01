//
// Copyright 2017-2018 Timo Kloss
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

#ifndef runner_h
#define runner_h

#include <stdio.h>
#include <stdbool.h>
#include "core.h"

struct Runner {
    struct Core *core;
    struct CoreDelegate coreDelegate;
    bool messageShownUsingDisk;
};

void runner_init(struct Runner *runner);
void runner_deinit(struct Runner *runner);
bool runner_isOkay(struct Runner *runner);
struct CoreError runner_loadProgram(struct Runner *runner, const char *filename);

#endif /* runner_h */
