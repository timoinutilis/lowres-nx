//
// Copyright 2016-2020 Timo Kloss
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

#ifndef core_h
#define core_h

#define CORE_VERSION "1.2"

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
    bool out_hasUsedInput;
};

extern const char CoreInputKeyReturn;
extern const char CoreInputKeyBackspace;
extern const char CoreInputKeyRight;
extern const char CoreInputKeyLeft;
extern const char CoreInputKeyDown;
extern const char CoreInputKeyUp;

void core_init(struct Core *core);
void core_deinit(struct Core *core);
void core_setDelegate(struct Core *core, struct CoreDelegate *delegate);
struct CoreError core_compileProgram(struct Core *core, const char *sourceCode, bool resetPersistent);
void core_traceError(struct Core *core, struct CoreError error);
void core_willRunProgram(struct Core *core, long secondsSincePowerOn);
void core_update(struct Core *core, struct CoreInput *input);
void core_willSuspendProgram(struct Core *core);
void core_setDebug(struct Core *core, bool enabled);
bool core_getDebug(struct Core *core);
bool core_isKeyboardEnabled(struct Core *core);
bool core_shouldRender(struct Core *core);

void core_setInputGamepad(struct CoreInput *input, int player, bool up, bool down, bool left, bool right, bool buttonA, bool buttonB);

void core_diskLoaded(struct Core *core);

// for dev mode only:
void core_handleInput(struct Core *core, struct CoreInput *input);

#endif /* core_h */
