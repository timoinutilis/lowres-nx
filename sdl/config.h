//
// Copyright 2018 Timo Kloss
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

#ifndef config_h
#define config_h

#ifdef __EMSCRIPTEN__
#define DEV_MENU 0
#define SCREENSHOTS 0
#define HOT_KEYS 0
#define SETTINGS_FILE 0
#else
#define DEV_MENU 1
#define SCREENSHOTS 1
#define HOT_KEYS 1
#define SETTINGS_FILE 1
#endif

#endif /* config_h */
