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

#ifndef overlay_h
#define overlay_h

#include <stdio.h>
#include <stdbool.h>
#include "video_chip.h"
#include "overlay_data.h"

#define MAX_BUTTONS 7

struct Core;

struct OverlayTouch {
    bool touched;
    int x;
    int y;
    int currentButton;
};

enum OverlayButtonType {
    OverlayButtonTypeDPad,
    OverlayButtonTypeA,
    OverlayButtonTypeB,
    OverlayButtonTypePause,
};

struct OverlayButton {
    enum OverlayButtonType type;
    int x;
    int y;
    int player;
};

struct Overlay {
    struct Plane plane;
    struct OverlayTouch touch;
    struct OverlayButton buttons[MAX_BUTTONS];
    int numButtons;
};

void overlay_init(struct Core *core);
void overlay_updateButtonConfiguration(struct Core *core);
void overlay_drawButtons(struct Core *core);
void overlay_touchPressed(struct Core *core, int x, int y);
void overlay_touchDragged(struct Core *core, int x, int y);
void overlay_touchReleased(struct Core *core);

#endif /* overlay_h */
