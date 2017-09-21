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

#include "overlay.h"
#include "core.h"
#include "io_chip.h"
#include <math.h>

void overlay_clear(struct Core *core);


void overlay_init(struct Core *core)
{
    overlay_updateButtonConfiguration(core);
}

void overlay_updateButtonConfiguration(struct Core *core)
{
    struct OverlayButton *buttons = core->overlay.buttons;
    
    int numOverlayGamepads = (int)core->machine.ioRegisters.attr.gamepadsEnabled - core->numPhysicalGamepads;
    
    if (numOverlayGamepads == 1)
    {
        int player = core->numPhysicalGamepads;
        
        core->overlay.numButtons = 4;
        
        buttons[0].type = OverlayButtonTypeDPad;
        buttons[0].x = 1;
        buttons[0].y = 12;
        buttons[0].player = player;
        
        buttons[1].type = OverlayButtonTypeA;
        buttons[1].x = 15;
        buttons[1].y = 13;
        buttons[1].player = player;
        
        buttons[2].type = OverlayButtonTypeB;
        buttons[2].x = 17;
        buttons[2].y = 12;
        buttons[2].player = player;
        
        buttons[3].type = OverlayButtonTypePause;
        buttons[3].x = 12;
        buttons[3].y = 14;
        buttons[3].player = player;
    }
    else if (numOverlayGamepads == 2)
    {
        core->overlay.numButtons = 6;
        
        buttons[0].type = OverlayButtonTypeDPad;
        buttons[0].x = 1;
        buttons[0].y = 12;
        buttons[0].player = 0;
        
        buttons[1].type = OverlayButtonTypeA;
        buttons[1].x = 5;
        buttons[1].y = 13;
        buttons[1].player = 0;
        
        buttons[2].type = OverlayButtonTypeB;
        buttons[2].x = 7;
        buttons[2].y = 12;
        buttons[2].player = 0;
        
        buttons[3].type = OverlayButtonTypeDPad;
        buttons[3].x = 11;
        buttons[3].y = 12;
        buttons[3].player = 1;
        
        buttons[4].type = OverlayButtonTypeA;
        buttons[4].x = 15;
        buttons[4].y = 13;
        buttons[4].player = 1;
        
        buttons[5].type = OverlayButtonTypeB;
        buttons[5].x = 17;
        buttons[5].y = 12;
        buttons[5].player = 1;
    }
    else
    {
        core->overlay.numButtons = 0;
    }
    
    overlay_clear(core);
}

void overlay_updateState(struct Core *core)
{
    if (core->interpreter.state == StatePaused)
    {
        core->overlay.numButtons = 0;
        overlay_clear(core);
        core->overlay.timer = 0;
    }
    else
    {
        overlay_updateButtonConfiguration(core);
    }
}

void overlay_drawDPad(struct Plane *plane, int x, int y, union Gamepad gamepad)
{
    plane->cells[y][x].character = 16;
    plane->cells[y][x+1].character = gamepad.up ? 20 : 17;
    plane->cells[y][x+2].character = 18;
    plane->cells[y+1][x].character = gamepad.left ? 35 : 32;
    plane->cells[y+1][x+1].character = 33;
    plane->cells[y+1][x+2].character = gamepad.right ? 37 : 34;
    plane->cells[y+2][x].character = 48;
    plane->cells[y+2][x+1].character = gamepad.down ? 52 : 49;
    plane->cells[y+2][x+2].character = 50;
}

void overlay_drawButton(struct Plane *plane, int x, int y, int character)
{
    plane->cells[y][x].character = character;
    plane->cells[y][x+1].character = character + 1;
    plane->cells[y+1][x].character = character + 16;
    plane->cells[y+1][x+1].character = character + 17;
}

void overlay_drawButtons(struct Core *core)
{
    struct Plane *plane = &core->overlay.plane;
    
    for (int i = 0; i < core->overlay.numButtons; i++)
    {
        struct OverlayButton *button = &core->overlay.buttons[i];
        union Gamepad gamepad = core->machine.ioRegisters.gamepads[button->player];
        switch (button->type)
        {
            case OverlayButtonTypeDPad:
                overlay_drawDPad(plane, button->x, button->y, gamepad);
                break;
                
            case OverlayButtonTypeA:
                overlay_drawButton(plane, button->x, button->y, gamepad.buttonA ? 40 : 38);
                break;
                
            case OverlayButtonTypeB:
                overlay_drawButton(plane, button->x, button->y, gamepad.buttonB ? 44 : 42);
                break;
                
            case OverlayButtonTypePause:
                overlay_drawButton(plane, button->x, button->y, core->machine.ioRegisters.status.pause ? 8 : 6);
                break;
        }
    }
}

void overlay_draw(struct Core *core)
{
    if (core->interpreter.state == StatePaused)
    {
        struct TextLib *lib = &core->interpreter.textLib;
        struct TextLib userTextLib = *lib;
        lib->bg = 2;
        lib->fontCharAttr.priority = 1;
        lib->fontCharAttr.palette = 1;
        lib->fontCharOffset = 64;
        if (core->overlay.timer % 30 < 20)
        {
            txtlib_writeText(core, "PAUSED", 7, 7);
        }
        else
        {
            txtlib_writeText(core, "      ", 7, 7);
        }
        core->interpreter.textLib = userTextLib;
    }
    else
    {
        overlay_drawButtons(core);
    }
    core->overlay.timer++;
}

void overlay_clear(struct Core *core)
{
    struct Plane *plane = &core->overlay.plane;
    for (int y = 0; y < PLANE_ROWS; y++)
    {
        for (int x = 0; x < PLANE_COLUMNS; x++)
        {
            struct Cell *cell = &plane->cells[y][x];
            cell->character = 0;
            cell->attr.palette = 0;
            cell->attr.priority = 1;
        }
    }
}

bool overlay_isInsideButton(struct OverlayButton *button, int x, int y)
{
    int pixelX = button->x << 3;
    int pixelY = button->y << 3;
    if (button->type == OverlayButtonTypeDPad)
    {
        return (x >= pixelX - 8 && y >= pixelY - 8 && x < pixelX + 32 && y < pixelY + 32);
    }
    else
    {
        return (x >= pixelX && y >= pixelY && x < pixelX + 16 && y < pixelY + 16);
    }
}

void overlay_handleDPad(union Gamepad *gamepad, struct OverlayButton *button, int x, int y)
{
    float diffX = x - (button->x << 3) - 12;
    float diffY = y - (button->y << 3) - 12;
    
    gamepad->up = (diffY < -3.0) && fabsf(diffX / diffY) < 2.0 ? 1 : 0;
    gamepad->down = (diffY > 3.0) && fabsf(diffX / diffY) < 2.0 ? 1 : 0;
    gamepad->left = (diffX < -3.0) && fabsf(diffY / diffX) < 2.0 ? 1 : 0;
    gamepad->right = (diffX > 3.0) && fabsf(diffY / diffX) < 2.0 ? 1 : 0;
}

struct OverlayTouch *overlay_getTouch(struct Core *core, const void *touchReference)
{
    for (int i = 0; i < MAX_TOUCHES; i++)
    {
        struct OverlayTouch *touch = &core->overlay.touch[i];
        if (touch->reference == touchReference)
        {
            return touch;
        }
    }
    return NULL;
}

void overlay_touchPressed(struct Core *core, int x, int y, const void *touchReference)
{
    for (int i = 0; i < core->overlay.numButtons; i++)
    {
        struct OverlayButton *button = &core->overlay.buttons[i];
        if (overlay_isInsideButton(button, x, y))
        {
            struct OverlayTouch *touch = overlay_getTouch(core, NULL);
            if (!touch) return;
            
            touch->reference = touchReference;
            touch->x = x;
            touch->y = y;
            touch->touched = true;
            touch->currentButton = i;
            
            union Gamepad *gamepad = &core->machine.ioRegisters.gamepads[button->player];
            switch (button->type)
            {
                case OverlayButtonTypeDPad:
                    overlay_handleDPad(gamepad, button, x, y);
                    break;
                    
                case OverlayButtonTypeA:
                    gamepad->buttonA = 1;
                    break;
                    
                case OverlayButtonTypeB:
                    gamepad->buttonB = 1;
                    break;
                    
                case OverlayButtonTypePause:
                    core->machine.ioRegisters.status.pause = 1;
                    break;
            }
            break;
        }
    }
}

void overlay_touchDragged(struct Core *core, int x, int y, const void *touchReference)
{
    struct OverlayTouch *touch = overlay_getTouch(core, touchReference);
    if (touch && touch->touched)
    {
        struct OverlayButton *button = &core->overlay.buttons[touch->currentButton];
        if (button->type == OverlayButtonTypeDPad)
        {
            union Gamepad *gamepad = &core->machine.ioRegisters.gamepads[button->player];
            overlay_handleDPad(gamepad, button, x, y);
        }
    }
}

void overlay_touchReleased(struct Core *core, const void *touchReference)
{
    struct OverlayTouch *touch = overlay_getTouch(core, touchReference);
    if (!touch) return;
    
    if (touch->touched)
    {
        struct OverlayButton *button = &core->overlay.buttons[touch->currentButton];
        
        union Gamepad *gamepad = &core->machine.ioRegisters.gamepads[button->player];
        switch (button->type)
        {
            case OverlayButtonTypeDPad:
                gamepad->up = 0;
                gamepad->down = 0;
                gamepad->left = 0;
                gamepad->right = 0;
                break;
                
            case OverlayButtonTypeA:
                gamepad->buttonA = 0;
                break;
                
            case OverlayButtonTypeB:
                gamepad->buttonB = 0;
                break;
                
            case OverlayButtonTypePause:
                core->machine.ioRegisters.status.pause = 0;
                break;
        }
        
        touch->touched = false;
    }
    
    touch->reference = NULL;
}
