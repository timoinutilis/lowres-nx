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

#include "runner.h"
#include "main.h"
#include "sdl_include.h"
#include "system_paths.h"
#include <string.h>
#include <stdlib.h>

void interpreterDidFail(void *context, struct CoreError coreError);
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager);
void diskDriveDidSave(void *context, struct DataManager *diskDataManager);
void controlsDidChange(void *context, struct ControlsInfo controlsInfo);
void persistentRamWillAccess(void *context, uint8_t *destination, int size);
void persistentRamDidChange(void *context, uint8_t *data, int size);


void runner_init(struct Runner *runner)
{
    memset(runner, 0, sizeof(struct Runner));
    
    struct Core *core = calloc(1, sizeof(struct Core));
    if (core)
    {
        core_init(core);
        
        runner->coreDelegate.context = runner;
        runner->coreDelegate.interpreterDidFail = interpreterDidFail;
        runner->coreDelegate.diskDriveWillAccess = diskDriveWillAccess;
        runner->coreDelegate.diskDriveDidSave = diskDriveDidSave;
        runner->coreDelegate.controlsDidChange = controlsDidChange;
        runner->coreDelegate.persistentRamWillAccess = persistentRamWillAccess;
        runner->coreDelegate.persistentRamDidChange = persistentRamDidChange;
        
        core_setDelegate(core, &runner->coreDelegate);

        runner->core = core;
    }
}

void runner_deinit(struct Runner *runner)
{
    if (runner->core)
    {
        core_deinit(runner->core);
        
        free(runner->core);
        runner->core = NULL;
    }
}

bool runner_isOkay(struct Runner *runner)
{
    return (runner->core != NULL);
}

struct CoreError runner_loadProgram(struct Runner *runner, const char *filename)
{
    struct CoreError error = err_noCoreError();
    
    FILE *file = fopen_utf8(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char *sourceCode = calloc(1, size + 1); // +1 for terminator
        if (sourceCode)
        {
            fread(sourceCode, size, 1, file);
            
            error = core_compileProgram(runner->core, sourceCode);
            free(sourceCode);
        }
        else
        {
            error = err_makeCoreError(ErrorOutOfMemory, -1);
        }
        
        fclose(file);
    }
    else
    {
        error = err_makeCoreError(ErrorCouldNotOpenProgram, -1);
    }
    
    return error;
}

/** Called on error */
void interpreterDidFail(void *context, struct CoreError coreError)
{
    struct Runner *runner = context;
    core_traceError(runner->core, coreError);
}

/** Returns true if the disk is ready, false if not. In case of not, core_diskLoaded must be called when ready. */
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager)
{
    struct Runner *runner = context;
    if (!runner->messageShownUsingDisk && !usesMainProgramAsDisk())
    {
#ifdef __EMSCRIPTEN__
        overlay_message(runner->core, "NO DISK");
#else
        overlay_message(runner->core, "USING DISK.NX");
#endif
        runner->messageShownUsingDisk = true;
    }
    
#ifndef __EMSCRIPTEN__
    
    char diskFilename[FILENAME_MAX];
    getDiskFilename(diskFilename);
    
    FILE *file = fopen_utf8(diskFilename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char *sourceCode = calloc(1, size + 1); // +1 for terminator
        if (sourceCode)
        {
            fread(sourceCode, size, 1, file);
            
            struct CoreError error = data_import(diskDataManager, sourceCode, true);
            free(sourceCode);
            
            if (error.code != ErrorNone)
            {
                core_traceError(runner->core, error);
            }
        }
        else
        {
            struct TextLib *lib = &runner->core->overlay->textLib;
            txtlib_printText(lib, "NOT ENOUGH MEMORY\n");
        }
        
        fclose(file);
    }
    
#endif
    
    return true;
}

/** Called when a disk data entry was saved */
void diskDriveDidSave(void *context, struct DataManager *diskDataManager)
{
    struct Runner *runner = context;
#ifdef __EMSCRIPTEN__
    overlay_message(runner->core, "NO DISK");
#else
    char *output = data_export(diskDataManager);
    if (output)
    {
        char diskFilename[FILENAME_MAX];
        getDiskFilename(diskFilename);
        
        FILE *file = fopen_utf8(diskFilename, "wb");
        if (file)
        {
            fwrite(output, 1, strlen(output), file);
            fclose(file);
        }
        else
        {
            struct TextLib *lib = &runner->core->overlay->textLib;
            txtlib_printText(lib, "COULD NOT SAVE:\n");
            txtlib_printText(lib, diskFilename);
            txtlib_printText(lib, "\n");
        }
        
        free(output);
    }
#endif
}

/** Called when keyboard or gamepad settings changed */
void controlsDidChange(void *context, struct ControlsInfo controlsInfo)
{
    if (   controlsInfo.keyboardMode == KeyboardModeOn
        || (controlsInfo.keyboardMode == KeyboardModeOptional && !SDL_HasScreenKeyboardSupport()) )
    {
        if (!SDL_IsTextInputActive())
        {
            SDL_StartTextInput();
        }
    }
    else if (SDL_IsTextInputActive())
    {
        SDL_StopTextInput();
    }
    setMouseEnabled(controlsInfo.isTouchEnabled);
}

/** Called when persistent RAM will be accessed the first time */
void persistentRamWillAccess(void *context, uint8_t *destination, int size)
{
#ifndef __EMSCRIPTEN__
    char ramFilename[FILENAME_MAX];
    getRamFilename(ramFilename);
    
    FILE *file = fopen_utf8(ramFilename, "rb");
    if (file)
    {
        fread(destination, sizeof(uint8_t), size, file);
        fclose(file);
    }
#endif
}

/** Called when persistent RAM should be saved */
void persistentRamDidChange(void *context, uint8_t *data, int size)
{
#ifndef __EMSCRIPTEN__
    struct Runner *runner = context;
    
    char ramFilename[FILENAME_MAX];
    getRamFilename(ramFilename);
    
    FILE *file = fopen_utf8(ramFilename, "wb");
    if (file)
    {
        fwrite(data, 1, size, file);
        fclose(file);
    }
    else
    {
        struct TextLib *lib = &runner->core->overlay->textLib;
        txtlib_printText(lib, "COULD NOT SAVE:\n");
        txtlib_printText(lib, ramFilename);
        txtlib_printText(lib, "\n");
    }
#endif
}
