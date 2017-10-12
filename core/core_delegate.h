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

struct ControlsInfo {
    bool isKeyboardEnabled;
    int numGamepadsEnabled;
};

struct CoreDelegate {
    void *context;
    void (*interpreterDidFail)(void *context, struct CoreError coreError);
    void (*diskDriveWillAccess)(void *context, struct DataManager *diskDataManager);
    void (*diskDriveDidSave)(void *context, struct DataManager *diskDataManager);
    void (*controlsDidChange)(void *context, struct ControlsInfo);
};

void delegate_interpreterDidFail(struct Core *core, struct CoreError coreError);
void delegate_diskDriveWillAccess(struct Core *core);
void delegate_diskDriveDidSave(struct Core *core);
void delegate_controlsDidChange(struct Core *core);

#endif /* core_delegate_h */
