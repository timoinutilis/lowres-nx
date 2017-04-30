//
// Copyright 2016 Timo Kloss
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

#ifndef core_h
#define core_h

#include <stdio.h>
#include "machine.h"
#include "interpreter.h"

struct Core {
    struct Machine machine;
    struct Interpreter interpreter;
};

enum GamepadButton {
    GamepadButtonUp,
    GamepadButtonDown,
    GamepadButtonLeft,
    GamepadButtonRight,
    GamepadButtonA,
    GamepadButtonB
};

void core_init(struct Core *core);
void core_update(struct Core *core);
void core_rasterUpdate(struct Core *core);

void core_keyPressed(struct Core *core, char key);
void core_backspacePressed(struct Core *core);
void core_returnPressed(struct Core *core);
void core_mouseMoved(struct Core *core, int x, int y);
void core_mousePressed(struct Core *core);
void core_mouseReleased(struct Core *core);
void core_gamepadPressed(struct Core *core, int player, enum GamepadButton button);
void core_gamepadReleased(struct Core *core, int player, enum GamepadButton button);

#endif /* core_h */
