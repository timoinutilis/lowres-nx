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

#include "dev_mode.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "dev_mode_data.h"
#include "text_lib.h"
#include "string_utils.h"

#ifdef __EMSCRIPTEN__
#include <SDL2/SDL.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <SDL2/SDL.h>
#elif defined(__LINUX__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#ifdef _WIN32
const char pathSeparator = '\\';
#else
const char pathSeparator = '/';
#endif

struct DevButton {
    int cx;
    int cy;
};

struct DevButton devButtons[] = {
    {1,4},
    {3,4},
    {5,4},
    {7,4},
    {17,4}
};

const char *devTools[] = {
    "Character Designer 1.1.nx",
    "Background Designer 1.1.nx",
    "Cancel"
};

void dev_showInfo(struct DevMode *devMode);
void dev_showError(struct DevMode *devMode, struct CoreError error);
void dev_updateButtons(struct DevMode *devMode);
void dev_onButtonTap(struct DevMode *devMode);
void dev_reloadProgram(struct DevMode *devMode);
void dev_runToolProgram(struct DevMode *devMode, const char *filename);
struct CoreError dev_loadProgram(struct DevMode *devMode, const char *filename);
void dev_showMenu(struct DevMode *devMode, const char *message, const char *buttons[], int numButtons);

bool dev_hasProgram(struct DevMode *devMode)
{
    return devMode->mainProgramFilename[0] != 0;
}

void dev_show(struct DevMode *devMode)
{
    if (devMode->state == DevModeStateRunningTool)
    {
        // did run tool, so reload main program
        dev_reloadProgram(devMode);
    }
    
    devMode->state = DevModeStateVisible;
    devMode->currentMenu = DevModeMenuMain;
    devMode->currentButton = -1;
    devMode->lastTouch = false;
    
    struct Core *core = devMode->core;
    
    struct TextLib *textLib = &devMode->textLib;
    textLib->core = core;
    
    core->interpreter->state = StateEnd;
    machine_reset(core);
    overlay_reset(core);
    
    txtlib_clearScreen(textLib);
    textLib->fontCharOffset = 192;
    textLib->charAttrFilter = 0xFF;
    textLib->windowY = 7;
    textLib->windowHeight = 9;
    
    memcpy(&core->machine->colorRegisters, dev_colors, sizeof(dev_colors));
    memcpy(&core->machine->videoRam.characters, dev_characters, sizeof(dev_characters));
    memcpy(&core->machine->cartridgeRom, dev_bg, sizeof(dev_bg));
    
    textLib->sourceAddress = 4;
    textLib->sourceWidth = core->machine->cartridgeRom[2];
    
    txtlib_copyBackground(textLib, 0, 0, 20, 16, 0, 0);
    dev_updateButtons(devMode);
    
    textLib->charAttr.palette = 1;
    txtlib_writeText(textLib, "DEVELOPMENT MENU", 2, 0);
    
    textLib->charAttr.palette = 0;
    char progName[19];
    memset(progName, 0, 19);
    char *slash = strrchr(devMode->mainProgramFilename, pathSeparator);
    if (slash)
    {
        strncpy(progName, slash + 1, 18);
    }
    else
    {
        strncpy(progName, devMode->mainProgramFilename, 18);
    }
    txtlib_writeText(textLib, progName, 1, 2);
    
    if (devMode->lastError.code != ErrorNone)
    {
        dev_showError(devMode, devMode->lastError);
    }
    else
    {
        dev_showInfo(devMode);
    }
}

void dev_update(struct DevMode *devMode, struct CoreInput *input)
{
    struct Core *core = devMode->core;
    struct TextLib *textLib = &devMode->textLib;
    
    core_handleInput(core, input);
    
    bool touch = core->machine->ioRegisters.status.touch;
    int cx = core->machine->ioRegisters.touchX / 8;
    int cy = core->machine->ioRegisters.touchY / 8;
    
    if (devMode->currentMenu == DevModeMenuMain)
    {
        if (devMode->currentButton >= 0)
        {
            int bcx = devButtons[devMode->currentButton].cx;
            int bcy = devButtons[devMode->currentButton].cy;
            bool isInside = (cx >= bcx && cy >= bcy && cx <= bcx + 1 && cy <= bcy + 1);
            if (!touch || !isInside)
            {
                textLib->charAttr.palette = 0;
                txtlib_setCells(textLib, bcx, bcy, bcx + 1, bcy + 1, -1);
                
                if (isInside)
                {
                    dev_onButtonTap(devMode);
                }
                devMode->currentButton = -1;
            }
        }
        else if (touch && !devMode->lastTouch)
        {
            for (int i = 0; i < 5; i++)
            {
                int bcx = devButtons[i].cx;
                int bcy = devButtons[i].cy;
                if (cx >= bcx && cy >= bcy && cx <= bcx + 1 && cy <= bcy + 1)
                {
                    textLib->charAttr.palette = 1;
                    txtlib_setCells(textLib, bcx, bcy, bcx + 1, bcy + 1, -1);
                    devMode->currentButton = i;
                }
            }
        }
    }
    else
    {
        if (devMode->currentButton >= 0)
        {
            int bcy = 1 + devMode->currentButton * 3;
            bool isInside = (cy >= bcy && cy <= bcy + 2);
            if (!touch || !isInside)
            {
                textLib->charAttr.palette = 0;
                txtlib_setCells(textLib, 0, bcy, 19, bcy + 2, -1);
                
                if (isInside)
                {
                    dev_onButtonTap(devMode);
                }
                devMode->currentButton = -1;
            }
        }
        else if (touch && !devMode->lastTouch)
        {
            int button = (cy - 1) / 3;
            if (button >= 0 && button < devMode->currentMenuSize)
            {
                int bcy = 1 + button * 3;
                textLib->charAttr.palette = 1;
                txtlib_setCells(textLib, 0, bcy, 19, bcy + 2, -1);
                devMode->currentButton = button;
            }
        }
    }
    devMode->lastTouch = core->machine->ioRegisters.status.touch;
    
    overlay_draw(core, false);
}

void dev_showInfo(struct DevMode *devMode)
{
    struct Core *core = devMode->core;
    struct TextLib *textLib = &devMode->textLib;
    
    char info[21];
    
    textLib->charAttr.palette = 5;
    txtlib_writeText(textLib, "TOKENS:", 0, 7);
    txtlib_writeText(textLib, "ROM:", 0, 8);

    textLib->charAttr.palette = 0;
    sprintf(info, "%d/%d", core->interpreter->tokenizer.numTokens, MAX_TOKENS);
    txtlib_writeText(textLib, info, 20 - (int)strlen(info), 7);
    sprintf(info, "%d/%d", data_currentSize(&core->interpreter->romDataManager), DATA_SIZE);
    txtlib_writeText(textLib, info, 20 - (int)strlen(info), 8);
    
    textLib->charAttr.palette = 4;
    txtlib_writeText(textLib, "READY TO RUN", 4, 14);
}

void dev_showError(struct DevMode *devMode, struct CoreError error)
{
    struct Core *core = devMode->core;
    struct TextLib *textLib = &devMode->textLib;
    
    textLib->charAttr.palette = 0;
    
    txtlib_clearWindow(textLib);
    
    textLib->charAttr.palette = 2;
    txtlib_printText(textLib, err_getString(error.code));
    txtlib_printText(textLib, "\n");
    if (error.sourcePosition >= 0 && core->interpreter->sourceCode)
    {
        textLib->charAttr.palette = 0;
        int number = lineNumber(core->interpreter->sourceCode, error.sourcePosition);
        char lineNumberText[30];
        sprintf(lineNumberText, "IN LINE %d:\n", number);
        txtlib_printText(textLib, lineNumberText);
        
        const char *line = lineString(core->interpreter->sourceCode, error.sourcePosition);
        if (line)
        {
            textLib->charAttr.palette = 5;
            txtlib_printText(textLib, "\n");
            txtlib_printText(textLib, line);
            free((void *)line);
        }
    }
}

void dev_updateButtons(struct DevMode *devMode)
{
    struct Plane *bg = &devMode->core->machine->videoRam.planeA;
    if (devMode->core->interpreter->debug)
    {
        bg->cells[5][7].character = 30;
        bg->cells[5][8].character = 31;
    }
    else
    {
        bg->cells[5][7].character = 46;
        bg->cells[5][8].character = 47;
    }
}

void dev_onButtonTap(struct DevMode *devMode)
{
    int button = devMode->currentButton;
    
    if (devMode->currentMenu == DevModeMenuMain)
    {
        if (button == 0)
        {
            // Run
            dev_runProgram(devMode);
        }
        else if (button == 1)
        {
            // Check
            dev_reloadProgram(devMode);
            dev_show(devMode);
        }
        else if (button == 2)
        {
            devMode->currentMenu = DevModeMenuTools;
            dev_showMenu(devMode, "EDIT ROM WITH TOOL", devTools, 3);
        }
        else if (button == 3)
        {
            // Debug On/Off
            devMode->core->interpreter->debug = !devMode->core->interpreter->debug;
            dev_updateButtons(devMode);
        }
        else if (button == 4)
        {
            // Exit
            devMode->state = DevModeStateOff;
        }
    }
    else if (devMode->currentMenu == DevModeMenuTools)
    {
        if (devMode->currentButton < 2)
        {
            const char *tool = devTools[devMode->currentButton];
            char toolFilename[FILENAME_MAX];
            strncpy(toolFilename, devMode->settings->programsPath, FILENAME_MAX - 1);
            strncat(toolFilename, tool, FILENAME_MAX - 1);
            dev_runToolProgram(devMode, toolFilename);
        }
        else
        {
            dev_show(devMode);
        }
    }
}

void dev_reloadProgram(struct DevMode *devMode)
{
    devMode->lastError = dev_loadProgram(devMode, devMode->mainProgramFilename);
}

void dev_runProgram(struct DevMode *devMode)
{
    dev_reloadProgram(devMode);
    if (devMode->lastError.code == ErrorNone)
    {
        devMode->state = DevModeStateRunningProgram;
        core_willRunProgram(devMode->core, SDL_GetTicks() / 1000);
    }
    else
    {
        dev_show(devMode);
    }
}

void dev_runToolProgram(struct DevMode *devMode, const char *filename)
{
    struct Core *core = devMode->core;
    struct CoreError error = dev_loadProgram(devMode, filename);
    if (error.code == ErrorNone)
    {
        devMode->state = DevModeStateRunningTool;
        core->interpreter->debug = false;
        core_willRunProgram(core, SDL_GetTicks() / 1000);
    }
    else
    {
        core_traceError(core, error);
    }
}

struct CoreError dev_loadProgram(struct DevMode *devMode, const char *filename)
{
    struct Core *core = devMode->core;
    
    struct CoreError error = err_noCoreError();
    
    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char *sourceCode = calloc(1, size + 1); // +1 for NULL terminator
        if (sourceCode)
        {
            fread(sourceCode, size, 1, file);
            
            error = core_compileProgram(core, sourceCode);
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

void dev_showMenu(struct DevMode *devMode, const char *message, const char *buttons[], int numButtons)
{
    struct TextLib *textLib = &devMode->textLib;
    
    textLib->charAttr.palette = 0;
    txtlib_setCells(textLib, 0, 0, 19, 15, 1);
    
    textLib->charAttr.palette = 1;
    txtlib_setCells(textLib, 0, 0, 19, 0, 192);
    txtlib_writeText(textLib, message, (int)(20 - strlen(message))/2, 0);
    
    textLib->charAttr.palette = 0;
    for (int i = 0; i < numButtons; i++)
    {
        int y = 1 + i * 3;
        txtlib_setCells(textLib, 0, y, 19, y, 3);
        txtlib_setCells(textLib, 0, y + 2, 19, y + 2, 5);
        int tx = (int)(20 - strlen(buttons[i])) / 2;
        if (tx < 0) tx = 0;
        txtlib_writeText(textLib, buttons[i], tx, y + 1);
    }
    devMode->currentMenuSize = numButtons;
}
