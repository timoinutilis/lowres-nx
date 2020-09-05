//
// Copyright 2017-2020 Timo Kloss
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

#ifndef interpreter_config_h
#define interpreter_config_h

#define MAX_TOKENS 16384
#define MAX_SYMBOLS 2048
#define MAX_LABEL_STACK_ITEMS 128
#define MAX_JUMP_LABEL_ITEMS 256
#define MAX_SUB_ITEMS 256
#define MAX_SIMPLE_VARIABLES 256
#define MAX_ARRAY_VARIABLES 256
#define SYMBOL_NAME_SIZE 21
#define MAX_ARRAY_DIMENSIONS 4
#define MAX_ARRAY_SIZE 32768
#define MAX_CYCLES_TOTAL_PER_FRAME 17556
#define MAX_CYCLES_PER_VBL 1140
#define MAX_CYCLES_PER_RASTER 51
#define TIMER_WRAP_VALUE 5184000

#endif /* interpreter_config_h */
