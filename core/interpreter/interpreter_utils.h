//
//  interpreter_utils.h
//  LowRes NX macOS
//
//  Created by Timo Kloss on 11/5/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

#ifndef interpreter_utils_h
#define interpreter_utils_h

#include <stdio.h>
#include <stdbool.h>
#include "value.h"
#include "video_chip.h"

struct Core;

struct TypedValue itp_evaluateCharAttributes(struct Core *core, union CharacterAttributes oldAttr);
struct TypedValue itp_evaluateDisplayAttributes(struct Core *core, union DisplayAttributes oldAttr);

#endif /* interpreter_utils_h */
