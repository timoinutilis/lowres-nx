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

#ifndef interpreter_config_h
#define interpreter_config_h

#define MAX_TOKENS 2048
#define MAX_SYMBOLS 128
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 128
#define MAX_SIMPLE_VARIABLES 128
#define MAX_ARRAY_VARIABLES 128
#define SYMBOL_NAME_SIZE 21
#define MAX_ARRAY_DIMENSIONS 4
#define MAX_ARRAY_SIZE 32768
#define MAX_ROM_DATA_ENTRIES 16
#define MAX_CYCLES_PER_FRAME 4096
#define MAX_CYCLES_PER_RASTER 128
#define MAX_CYCLES_PER_VBL 128

#endif /* interpreter_config_h */
