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
