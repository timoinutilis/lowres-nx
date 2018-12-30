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

#ifndef core_delegate_h
#define core_delegate_h

#include "data_manager.h"
#include "error.h"

struct Core;

enum KeyboardMode {
    KeyboardModeOff,
    KeyboardModeOn,
    KeyboardModeOptional
};

struct ControlsInfo {
    enum KeyboardMode keyboardMode;
    int numGamepadsEnabled;
    bool isAudioEnabled;
};

struct CoreDelegate {
    void *context;
    
    /** Called on error */
    void (*interpreterDidFail)(void *context, struct CoreError coreError);
    
    /** Returns true if the disk is ready, false if not. In case of not, core_diskLoaded must be called when ready. */
    bool (*diskDriveWillAccess)(void *context, struct DataManager *diskDataManager);
    
    /** Called when a disk data entry was saved */
    void (*diskDriveDidSave)(void *context, struct DataManager *diskDataManager);
    
    /** Called when keyboard or gamepad settings changed */
    void (*controlsDidChange)(void *context, struct ControlsInfo controlsInfo);
};

void delegate_interpreterDidFail(struct Core *core, struct CoreError coreError);
bool delegate_diskDriveWillAccess(struct Core *core);
void delegate_diskDriveDidSave(struct Core *core);
void delegate_controlsDidChange(struct Core *core);

#endif /* core_delegate_h */
