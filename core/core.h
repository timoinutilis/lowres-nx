//
// Copyright 2016-2018 Timo Kloss
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

#define CORE_VERSION "0.8 (5)"

#include <stdio.h>
#include <stdbool.h>
#include "machine.h"
#include "overlay.h"
#include "interpreter.h"
#include "disk_drive.h"
#include "core_delegate.h"

struct Core {
    struct Machine *machine;
    struct MachineInternals *machineInternals;
    struct Interpreter *interpreter;
    struct DiskDrive *diskDrive;
    struct Overlay *overlay;
    struct CoreDelegate *delegate;
};

struct CoreInputGamepad {
    bool up;
    bool down;
    bool left;
    bool right;
    bool buttonA;
    bool buttonB;
};

struct CoreInput {
    struct CoreInputGamepad gamepads[NUM_GAMEPADS];
    bool pause;
    int touchX;
    int touchY;
    bool touch;
    char key;
};

extern const char CoreInputKeyReturn;
extern const char CoreInputKeyBackspace;

void core_init(struct Core *core);
void core_deinit(struct Core *core);
void core_setDelegate(struct Core *core, struct CoreDelegate *delegate);
struct CoreError core_compileProgram(struct Core *core, const char *sourceCode);
void core_traceError(struct Core *core, struct CoreError error);
void core_willRunProgram(struct Core *core, long secondsSincePowerOn);
void core_update(struct Core *core, struct CoreInput *input);
void core_setDebug(struct Core *core, bool enabled);
bool core_getDebug(struct Core *core);
bool core_getKeyboardEnabled(struct Core *core);
int core_getNumGamepads(struct Core *core);

void core_setInputGamepad(struct CoreInput *input, int player, bool up, bool down, bool left, bool right, bool buttonA, bool buttonB);

void core_diskLoaded(struct Core *core);

// for dev mode only:
void core_handleInput(struct Core *core, struct CoreInput *input);

#endif /* core_h */
