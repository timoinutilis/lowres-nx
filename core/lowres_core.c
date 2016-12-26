//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#include "lowres_core.h"
#include <stdlib.h>

void LRC_init(LRCore *core)
{
    core->videoInterface.colors[0] = 3;
    core->videoInterface.colors[1] = 12;
    core->videoInterface.colors[2] = 48;
    core->videoInterface.colors[3] = 63;
    
    Window *window = &core->videoInterface.window;
    window->cells[0][0].character = 'S';
    window->cells[0][1].character = 'C';
    window->cells[0][2].character = 'O';
    window->cells[0][3].character = 'R';
    window->cells[0][4].character = 'E';
    window->cells[0][5].character = 255;
    window->cells[0][6].character = 254;
    window->cells[0][7].character = 253;
}

void LRC_update(LRCore *core)
{
    core->videoInterface.planes[0].scrollX--;
    core->videoInterface.planes[1].scrollY--;
    core->videoInterface.planes[rand()%2].cells[rand()%32][rand()%32].character = 32+(rand()%32);
}
