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

#include "config.h"

#if DEV_MENU

#include "dev_menu.h"
#include "main.h"
#include "dev_menu_data.h"
#include "text_lib.h"
#include "string_utils.h"
#include "system_paths.h"
#include "utils.h"
#include "sdl_include.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MENU_SIZE 5

struct DevButton {
    int cx;
    int cy;
};

struct DevButton devButtons[] = {
    {1,4},
    {3,4},
    {5,4},
    {7,4},
    {9,4},
    {17,4}
};

void dev_showInfo(struct DevMenu *devMenu);
void dev_showError(struct DevMenu *devMenu, struct CoreError error);
void dev_updateButtons(struct DevMenu *devMenu);
void dev_onButtonTap(struct DevMenu *devMenu);
void dev_showToolsMenu(struct DevMenu *devMenu);
void dev_showClearRamMenu(struct DevMenu *devMenu);
void dev_showMenu(struct DevMenu *devMenu, const char *message, const char *buttons[], int numButtons, int numRemoveButtons);
void dev_clearPersistentRam(struct DevMenu *devMenu);

void dev_init(struct DevMenu *devMenu, struct Runner *runner, struct Settings *settings)
{
    memset(devMenu, 0, sizeof(struct DevMenu));
    devMenu->runner = runner;
    devMenu->settings = settings;
}

void dev_show(struct DevMenu *devMenu, bool reload)
{
    if (reload)
    {
        devMenu->lastError = runner_loadProgram(devMenu->runner, getMainProgramFilename());
    }
    
    devMenu->currentMenu = DevModeMenuMain;
    devMenu->currentButton = -1;
    devMenu->lastTouch = false;
    
    struct Core *core = devMenu->runner->core;
    
    struct TextLib *textLib = &devMenu->textLib;
    textLib->core = core;
    
    itp_endProgram(core);
    machine_reset(core);
    overlay_reset(core);
    
    core->machine->ioRegisters.attr.touchEnabled = 1;
    core->machineInternals->isEnergySaving = true;
    
    txtlib_clearScreen(textLib);
    textLib->fontCharOffset = 192;
    textLib->windowY = 7;
    textLib->windowHeight = 9;
    
    memcpy(&core->machine->colorRegisters, dev_colors, sizeof(dev_colors));
    memcpy(&core->machine->videoRam.characters, dev_characters, sizeof(dev_characters));
    memcpy(&core->machine->cartridgeRom, dev_bg, sizeof(dev_bg));
    
    textLib->sourceAddress = 4;
    textLib->sourceWidth = core->machine->cartridgeRom[2];
    
    txtlib_copyBackground(textLib, 0, 0, 20, 16, 0, 0);
    dev_updateButtons(devMenu);
    
    textLib->charAttr.palette = 1;
    txtlib_writeText(textLib, "DEVELOPMENT MENU", 2, 0);
    
    textLib->charAttr.palette = 0;
    char progName[19];
    displayName(getMainProgramFilename(), progName, 19);
    txtlib_writeText(textLib, progName, 1, 2);
    
    if (devMenu->lastError.code != ErrorNone)
    {
        dev_showError(devMenu, devMenu->lastError);
    }
    else
    {
        dev_showInfo(devMenu);
    }
    
    setMouseEnabled(true);
}

void dev_update(struct DevMenu *devMenu, struct CoreInput *input)
{
    struct Core *core = devMenu->runner->core;
    struct TextLib *textLib = &devMenu->textLib;
    
    core_handleInput(core, input);
    
    bool touch = core->machine->ioRegisters.status.touch;
    int cx = core->machine->ioRegisters.touchX / 8;
    int cy = core->machine->ioRegisters.touchY / 8;
    
    if (devMenu->currentMenu == DevModeMenuMain)
    {
        if (devMenu->currentButton >= 0)
        {
            int bcx = devButtons[devMenu->currentButton].cx;
            int bcy = devButtons[devMenu->currentButton].cy;
            bool isInside = (cx >= bcx && cy >= bcy && cx <= bcx + 1 && cy <= bcy + 1);
            if (!touch || !isInside)
            {
                txtlib_setCellsAttr(textLib, bcx, bcy, bcx + 1, bcy + 1, 0, -1, -1, -1);
                
                if (isInside)
                {
                    dev_onButtonTap(devMenu);
                }
                devMenu->currentButton = -1;
            }
        }
        else if (touch && !devMenu->lastTouch)
        {
            for (int i = 0; i < 6; i++)
            {
                int bcx = devButtons[i].cx;
                int bcy = devButtons[i].cy;
                if (cx >= bcx && cy >= bcy && cx <= bcx + 1 && cy <= bcy + 1)
                {
                    txtlib_setCellsAttr(textLib, bcx, bcy, bcx + 1, bcy + 1, 1, -1, -1, -1);
                    devMenu->currentButton = i;
                }
            }
        }
    }
    else
    {
        if (devMenu->currentButton >= 0)
        {
            int bcy = 1 + devMenu->currentButton * 3;
            bool isInside = (cy >= bcy && cy <= bcy + 2);
            if (!touch || !isInside)
            {
                txtlib_setCellsAttr(textLib, 0, bcy, 19, bcy + 2, 0, -1, -1, -1);
                
                if (isInside)
                {
                    dev_onButtonTap(devMenu);
                }
                devMenu->currentButton = -1;
            }
        }
        else if (touch && !devMenu->lastTouch)
        {
            int button = (cy - 1) / 3;
            if (button >= 0 && button < devMenu->currentMenuSize)
            {
                int bcy = 1 + button * 3;
                txtlib_setCellsAttr(textLib, 0, bcy, 19, bcy + 2, 1, -1, -1, -1);
                devMenu->currentButton = button;
            }
        }
    }
    devMenu->lastTouch = core->machine->ioRegisters.status.touch;
    
    overlay_draw(core, false);
}

bool dev_handleDropFile(struct DevMenu *devMenu, const char *filename)
{
    if (devMenu->currentMenu == DevModeMenuTools)
    {
        if (settings_addTool(devMenu->settings, filename))
        {
            settings_save(devMenu->settings);
            dev_showToolsMenu(devMenu);
        }
        else
        {
            overlay_message(devMenu->runner->core, "NO EMPTY SPACE");
        }
        return true;
    }
    return false;
}

void dev_showInfo(struct DevMenu *devMenu)
{
    struct Core *core = devMenu->runner->core;
    struct TextLib *textLib = &devMenu->textLib;
    
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

void dev_showError(struct DevMenu *devMenu, struct CoreError error)
{
    struct Core *core = devMenu->runner->core;
    struct TextLib *textLib = &devMenu->textLib;
    
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

void dev_updateButtons(struct DevMenu *devMenu)
{
    struct Plane *bg = &devMenu->runner->core->machine->videoRam.planeA;
    if (devMenu->runner->core->interpreter->debug)
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

void dev_onButtonTap(struct DevMenu *devMenu)
{
    int button = devMenu->currentButton;
    
    if (devMenu->currentMenu == DevModeMenuMain)
    {
        if (button == 0)
        {
            // Run
            runMainProgram();
        }
        else if (button == 1)
        {
            // Check
            dev_show(devMenu, true);
        }
        else if (button == 2)
        {
            dev_showToolsMenu(devMenu);
        }
        else if (button == 3)
        {
            // Debug On/Off
            devMenu->runner->core->interpreter->debug = !devMenu->runner->core->interpreter->debug;
            dev_updateButtons(devMenu);
        }
        else if (button == 4)
        {
            dev_showClearRamMenu(devMenu);
        }
        else if (button == 5)
        {
            // Eject
            rebootNX();
        }
    }
    else if (devMenu->currentMenu == DevModeMenuTools)
    {
        if (devMenu->currentButton < devMenu->settings->numTools)
        {
            int cx = devMenu->runner->core->machine->ioRegisters.touchX / 8;
            if (cx >= 18)
            {
                settings_removeTool(devMenu->settings, devMenu->currentButton);
                settings_save(devMenu->settings);
                dev_showToolsMenu(devMenu);
            }
            else
            {
                runToolProgram(devMenu->settings->tools[devMenu->currentButton]);
            }
        }
        else
        {
            dev_show(devMenu, false);
        }
    }
    else if (devMenu->currentMenu == DevModeMenuClearRam)
    {
        if (devMenu->currentButton == 0)
        {
            dev_clearPersistentRam(devMenu);
        }
        dev_show(devMenu, false);
    }
}

void dev_showToolsMenu(struct DevMenu *devMenu)
{
    struct TextLib *textLib = &devMenu->textLib;
    
    devMenu->currentMenu = DevModeMenuTools;
    const char *menu[MENU_SIZE];
    int count = 0;
    for (int i = 0; i < devMenu->settings->numTools; i++)
    {
        menu[count++] = devMenu->settings->toolNames[i];
    }
    menu[count++] = "CANCEL";
    dev_showMenu(devMenu, "EDIT ROM WITH TOOL", menu, count, count - 1);
    if (count < MENU_SIZE)
    {
        textLib->charAttr.palette = 5;
        txtlib_writeText(textLib, "DRAG & DROP PROGRAM", 0, 14);
        txtlib_writeText(textLib, "TO ADD AS TOOL", 3, 15);
    }
}

void dev_showClearRamMenu(struct DevMenu *devMenu)
{
    struct TextLib *textLib = &devMenu->textLib;
    
    devMenu->currentMenu = DevModeMenuClearRam;
    const char *menu[MENU_SIZE];
    menu[0] = "CLEAR";
    menu[1] = "CANCEL";
    dev_showMenu(devMenu, "CLEAR PERSIST. RAM?", menu, 2, 0);
    
    textLib->charAttr.palette = 5;
    txtlib_writeText(textLib, "MAY DELETE DATA LIKE", 0, 12);
    txtlib_writeText(textLib, "GAME STATE OR", 3, 13);
    txtlib_writeText(textLib, "HIGH SCORES", 4, 14);
    txtlib_writeText(textLib, "OF THIS PROGRAM", 2, 15);
}

void dev_showMenu(struct DevMenu *devMenu, const char *message, const char *buttons[], int numButtons, int numRemoveButtons)
{
    struct TextLib *textLib = &devMenu->textLib;
    
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
        if (i < numRemoveButtons)
        {
            txtlib_setCell(textLib, 19, y, 20);
        }
    }
    devMenu->currentMenuSize = numButtons;
}

void dev_clearPersistentRam(struct DevMenu *devMenu)
{
    char ramFilename[FILENAME_MAX];
    getRamFilename(ramFilename);
    remove(ramFilename);
}

#endif

