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

#include "core.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "string_utils.h"
#include "startup_sequence.h"

const char CoreInputKeyReturn = '\n';
const char CoreInputKeyBackspace = '\b';
const char CoreInputKeyRight = 17;
const char CoreInputKeyLeft = 18;
const char CoreInputKeyDown = 19;
const char CoreInputKeyUp = 20;

void core_handleInput(struct Core *core, struct CoreInput *input);


void core_init(struct Core *core)
{
    memset(core, 0, sizeof(struct Core));
    
    core->machine = calloc(1, sizeof(struct Machine));
    if (!core->machine) exit(EXIT_FAILURE);
    
    core->machineInternals = calloc(1, sizeof(struct MachineInternals));
    if (!core->machineInternals) exit(EXIT_FAILURE);
    
    core->interpreter = calloc(1, sizeof(struct Interpreter));
    if (!core->interpreter) exit(EXIT_FAILURE);
    
    core->diskDrive = calloc(1, sizeof(struct DiskDrive));
    if (!core->diskDrive) exit(EXIT_FAILURE);
    
    core->overlay = calloc(1, sizeof(struct Overlay));
    if (!core->overlay) exit(EXIT_FAILURE);
    
    machine_init(core);
    itp_init(core);
    overlay_init(core);
    disk_init(core);
}

void core_deinit(struct Core *core)
{
    itp_deinit(core);
    disk_deinit(core);
    
    free(core->machine);
    core->machine = NULL;
    
    free(core->machineInternals);
    core->machineInternals = NULL;
    
    free(core->interpreter);
    core->interpreter = NULL;
    
    free(core->diskDrive);
    core->diskDrive = NULL;
    
    free(core->overlay);
    core->overlay = NULL;
}

void core_setDelegate(struct Core *core, struct CoreDelegate *delegate)
{
    core->delegate = delegate;
}

struct CoreError core_compileProgram(struct Core *core, const char *sourceCode, bool resetPersistent)
{
    machine_reset(core, resetPersistent);
    overlay_reset(core);
    disk_reset(core);
    return itp_compileProgram(core, sourceCode);
}

void core_traceError(struct Core *core, struct CoreError error)
{
    core->interpreter->debug = false;
    struct TextLib *lib = &core->overlay->textLib;
    txtlib_printText(lib, err_getString(error.code));
    txtlib_printText(lib, "\n");
    if (error.sourcePosition >= 0 && core->interpreter->sourceCode)
    {
        int number = lineNumber(core->interpreter->sourceCode, error.sourcePosition);
        char lineNumberText[30];
        sprintf(lineNumberText, "IN LINE %d:\n", number);
        txtlib_printText(lib, lineNumberText);
        
        const char *line = lineString(core->interpreter->sourceCode, error.sourcePosition);
        if (line)
        {
            txtlib_printText(lib, line);
            txtlib_printText(lib, "\n");
            free((void *)line);
        }
    }
}

void core_willRunProgram(struct Core *core, long secondsSincePowerOn)
{
    runStartupSequence(core);
    core->interpreter->timer = (float)(secondsSincePowerOn * 60 % TIMER_WRAP_VALUE);
    machine_suspendEnergySaving(core, 30);
    delegate_controlsDidChange(core);
}

void core_update(struct Core *core, struct CoreInput *input)
{
    core_handleInput(core, input);
    itp_runInterrupt(core, InterruptTypeVBL);
    itp_runProgram(core);
    itp_didFinishVBL(core);
    overlay_draw(core, true);
    audio_bufferRegisters(core);
}

void core_handleInput(struct Core *core, struct CoreInput *input)
{
    struct IORegisters *ioRegisters = &core->machine->ioRegisters;
    union IOAttributes ioAttr = ioRegisters->attr;
    
    bool processedOtherInput = false;
    
    if (input->key != 0)
    {
        if (ioAttr.keyboardEnabled)
        {
            char key = input->key;
            if (   (key >= 32 && key < 127)
                || key == CoreInputKeyBackspace || key == CoreInputKeyReturn
                || key == CoreInputKeyDown || key == CoreInputKeyUp || key == CoreInputKeyRight || key == CoreInputKeyLeft )
            {
                ioRegisters->key = key;
            }
        }
        input->key = 0;
        machine_suspendEnergySaving(core, 2);
    }
    
    if (input->touch)
    {
        if (ioAttr.touchEnabled)
        {
            ioRegisters->status.touch = 1;
            int x = input->touchX;
            int y = input->touchY;
            if (x < 0) x = 0; else if (x >= SCREEN_WIDTH) x = SCREEN_WIDTH - 1;
            if (y < 0) y = 0; else if (y >= SCREEN_HEIGHT) y = SCREEN_HEIGHT - 1;
            ioRegisters->touchX = x;
            ioRegisters->touchY = y;
        }
        else
        {
            ioRegisters->status.touch = 0;
        }
        machine_suspendEnergySaving(core, 2);
    }
    else
    {
        ioRegisters->status.touch = 0;
    }
    
    for (int i = 0; i < NUM_GAMEPADS; i++)
    {
        union Gamepad *gamepad = &ioRegisters->gamepads[i];
        if (ioAttr.gamepadsEnabled > i && !ioAttr.keyboardEnabled)
        {
            struct CoreInputGamepad *inputGamepad = &input->gamepads[i];
            gamepad->up = inputGamepad->up && !inputGamepad->down;
            gamepad->down = inputGamepad->down && !inputGamepad->up;
            gamepad->left = inputGamepad->left && !inputGamepad->right;
            gamepad->right = inputGamepad->right && !inputGamepad->left;
            gamepad->buttonA = inputGamepad->buttonA;
            gamepad->buttonB = inputGamepad->buttonB;
            
            if (inputGamepad->up || inputGamepad->down || inputGamepad->left || inputGamepad->right)
            {
                // some d-pad combinations are not registered as I/O, but mark them anyway.
                processedOtherInput = true;
            }
            
            if (gamepad->value)
            {
                machine_suspendEnergySaving(core, 2);
            }
        }
        else
        {
            gamepad->value = 0;
        }
    }
    
    if (input->pause)
    {
        if (core->interpreter->state == StatePaused)
        {
            core->interpreter->state = StateEvaluate;
            overlay_updateState(core);
            processedOtherInput = true;
        }
        else if (ioAttr.gamepadsEnabled > 0 && !ioAttr.keyboardEnabled) {
            ioRegisters->status.pause = 1;
        }
        input->pause = false;
    }
    
    input->out_hasUsedInput = processedOtherInput || ioRegisters->key || ioRegisters->status.value || ioRegisters->gamepads[0].value || ioRegisters->gamepads[1].value;
}

void core_willSuspendProgram(struct Core *core)
{
    if (core->machineInternals->hasChangedPersistent)
    {
        delegate_persistentRamDidChange(core, core->machine->persistentRam, PERSISTENT_RAM_SIZE);
        core->machineInternals->hasChangedPersistent = false;
    }
}

void core_setDebug(struct Core *core, bool enabled)
{
    core->interpreter->debug = enabled;
    overlay_updateState(core);
}

bool core_getDebug(struct Core *core)
{
    return core->interpreter->debug;
}

bool core_isKeyboardEnabled(struct Core *core)
{
    return core->machine->ioRegisters.attr.keyboardEnabled;
}

bool core_shouldRender(struct Core *core)
{
    enum State state = core->interpreter->state;
    bool shouldRender = (!core->machineInternals->isEnergySaving && state != StateEnd && state != StateNoProgram)
    || core->machineInternals->energySavingTimer > 0
    || core->machineInternals->energySavingTimer % 20 == 0;
    
    core->machineInternals->energySavingTimer--;
    return shouldRender;
}

void core_setInputGamepad(struct CoreInput *input, int player, bool up, bool down, bool left, bool right, bool buttonA, bool buttonB)
{
    struct CoreInputGamepad *gamepad = &input->gamepads[player];
    gamepad->up = up;
    gamepad->down = down;
    gamepad->left = left;
    gamepad->right = right;
    gamepad->buttonA = buttonA;
    gamepad->buttonB = buttonB;
}

void core_diskLoaded(struct Core *core)
{
    core->interpreter->state = StateEvaluate;
}
