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

#include <stdio.h>
#include "core.h"

void delegate_interpreterDidFail(struct Core *core, struct CoreError coreError)
{
    if (core->delegate->interpreterDidFail)
    {
        core->delegate->interpreterDidFail(core->delegate->context, coreError);
    }
}

bool delegate_diskDriveWillAccess(struct Core *core)
{
    if (core->delegate->diskDriveWillAccess)
    {
        return core->delegate->diskDriveWillAccess(core->delegate->context, &core->diskDrive->dataManager);
    }
    return true;
}

void delegate_diskDriveDidSave(struct Core *core)
{
    if (core->delegate->diskDriveDidSave)
    {
        core->delegate->diskDriveDidSave(core->delegate->context, &core->diskDrive->dataManager);
    }
}

void delegate_diskDriveIsFull(struct Core *core)
{
    if (core->delegate->diskDriveIsFull)
    {
        core->delegate->diskDriveIsFull(core->delegate->context, &core->diskDrive->dataManager);
    }
}

void delegate_controlsDidChange(struct Core *core)
{
    if (core->delegate->controlsDidChange)
    {
        struct ControlsInfo info;
        union IOAttributes ioAttr = core->machine->ioRegisters.attr;
        if (ioAttr.keyboardEnabled)
        {
            if (core->interpreter->isKeyboardOptional)
            {
                info.keyboardMode = KeyboardModeOptional;
            }
            else
            {
                info.keyboardMode = KeyboardModeOn;
            }
        }
        else
        {
            info.keyboardMode = KeyboardModeOff;
        }
        info.numGamepadsEnabled = ioAttr.keyboardEnabled ? 0 : ioAttr.gamepadsEnabled;
        info.isTouchEnabled = ioAttr.touchEnabled;
        info.isAudioEnabled = core->machineInternals->audioInternals.audioEnabled;
        core->delegate->controlsDidChange(core->delegate->context, info);
    }
}

void delegate_persistentRamWillAccess(struct Core *core, uint8_t *destination, int size)
{
    if (core->delegate->persistentRamWillAccess)
    {
        core->delegate->persistentRamWillAccess(core->delegate->context, destination, size);
    }
}

void delegate_persistentRamDidChange(struct Core *core, uint8_t *data, int size)
{
    if (core->delegate->persistentRamDidChange)
    {
        core->delegate->persistentRamDidChange(core->delegate->context, data, size);
    }
}
