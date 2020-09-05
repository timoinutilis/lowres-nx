//
// Copyright 2017 Timo Kloss
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

#include "data.h"
#include "interpreter.h"

void dat_nextData(struct Interpreter *interpreter)
{
    interpreter->currentDataValueToken++;
    if (interpreter->currentDataValueToken->type == TokenComma)
    {
        // value follows
        interpreter->currentDataValueToken++;
    }
    else
    {
        // next DATA line
        interpreter->currentDataToken = interpreter->currentDataToken->jumpToken;
        if (interpreter->currentDataToken)
        {
            interpreter->currentDataValueToken = interpreter->currentDataToken + 1; // after DATA
        }
        else
        {
            interpreter->currentDataValueToken = NULL;
        }
    }
}

void dat_restoreData(struct Interpreter *interpreter, struct Token *jumpToken)
{
    if (jumpToken)
    {
        struct Token *dataToken = interpreter->firstData;
        while (dataToken && dataToken < jumpToken)
        {
            dataToken = dataToken->jumpToken;
        }
        interpreter->currentDataToken = dataToken;
    }
    else
    {
        interpreter->currentDataToken = interpreter->firstData;
    }
    
    if (interpreter->currentDataToken)
    {
        interpreter->currentDataValueToken = interpreter->currentDataToken + 1; // after DATA
    }
    else
    {
        interpreter->currentDataValueToken = NULL;
    }
}
