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
