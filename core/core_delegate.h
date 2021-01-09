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
    bool isTouchEnabled;
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
    
    /** Called when a disk data entry was tried to be saved, but the disk is full */
    void (*diskDriveIsFull)(void *context, struct DataManager *diskDataManager);
    
    /** Called when keyboard or gamepad settings changed */
    void (*controlsDidChange)(void *context, struct ControlsInfo controlsInfo);
    
    /** Called when persistent RAM will be accessed the first time */
    void (*persistentRamWillAccess)(void *context, uint8_t *destination, int size);
    
    /** Called when persistent RAM should be saved */
    void (*persistentRamDidChange)(void *context, uint8_t *data, int size);
};

void delegate_interpreterDidFail(struct Core *core, struct CoreError coreError);
bool delegate_diskDriveWillAccess(struct Core *core);
void delegate_diskDriveDidSave(struct Core *core);
void delegate_diskDriveIsFull(struct Core *core);
void delegate_controlsDidChange(struct Core *core);
void delegate_persistentRamWillAccess(struct Core *core, uint8_t *destination, int size);
void delegate_persistentRamDidChange(struct Core *core, uint8_t *data, int size);

#endif /* core_delegate_h */
