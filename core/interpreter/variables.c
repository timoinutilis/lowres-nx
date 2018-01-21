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

#include "variables.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>

struct SimpleVariable *var_getSimpleVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, enum ValueType type, bool forWriting)
{
    struct SimpleVariable *variable = NULL;
    for (int i = 0; i < interpreter->numSimpleVariables; i++)
    {
        variable = &interpreter->simpleVariables[i];
        if (variable->symbolIndex == symbolIndex)
        {
            // variable found
            return variable;
        }
    }
    
    if (forWriting)
    {
        // create new variable
        if (interpreter->numSimpleVariables >= MAX_SIMPLE_VARIABLES)
        {
            *errorCode = ErrorOutOfMemory;
            return NULL;
        }
        variable = &interpreter->simpleVariables[interpreter->numSimpleVariables];
        interpreter->numSimpleVariables++;
        memset(variable, 0, sizeof(struct SimpleVariable));
        variable->symbolIndex = symbolIndex;
        variable->type = type;
        if (type == ValueTypeString)
        {
            // assign global NullString
            variable->v.stringValue = interpreter->nullString;
            rcstring_retain(variable->v.stringValue);
        }
        return variable;
    }
    else
    {
        *errorCode = ErrorVariableNotInitialized;
        return NULL;
    }
}

void var_freeSimpleVariables(struct Interpreter *interpreter)
{
    for (int i = 0; i < interpreter->numSimpleVariables; i++)
    {
        struct SimpleVariable *variable = &interpreter->simpleVariables[i];
        if (variable->type == ValueTypeString)
        {
            rcstring_release(variable->v.stringValue);
        }
    }
}

struct ArrayVariable *var_getArrayVariable(struct Interpreter *interpreter, int symbolIndex)
{
    struct ArrayVariable *variable = NULL;
    for (int i = 0; i < interpreter->numArrayVariables; i++)
    {
        variable = &interpreter->arrayVariables[i];
        if (variable->symbolIndex == symbolIndex)
        {
            // variable found
            return variable;
        }
    }
    return NULL;
}

union Value *var_getArrayValue(struct Interpreter *interpreter, struct ArrayVariable *variable, int *indices)
{
    int offset = 0;
    int factor = 1;
    for (int i = variable->numDimensions - 1; i >= 0; i--)
    {
        offset += indices[i] * factor;
        factor *= variable->dimensionSizes[i];
    }
    union Value *value = &variable->values[offset];
    if (variable->type == ValueTypeString && !value->stringValue)
    {
        // string variable was still uninitialized, assign global NullString
        value->stringValue = interpreter->nullString;
        rcstring_retain(value->stringValue);
    }
    return value;
}

struct ArrayVariable *var_dimVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int numDimensions, int *dimensionSizes)
{
    if (var_getArrayVariable(interpreter, symbolIndex))
    {
        *errorCode = ErrorArrayAlreadyDimensionized;
        return NULL;
    }
    if (interpreter->numArrayVariables >= MAX_ARRAY_VARIABLES)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    struct ArrayVariable *variable = &interpreter->arrayVariables[interpreter->numArrayVariables];
    interpreter->numArrayVariables++;
    memset(variable, 0, sizeof(struct ArrayVariable));
    variable->symbolIndex = symbolIndex;
    variable->numDimensions = numDimensions;
    size_t size = 1;
    for (int i = 0; i < numDimensions; i++)
    {
        size *= dimensionSizes[i];
        variable->dimensionSizes[i] = dimensionSizes[i];
    }
    if (size > MAX_ARRAY_SIZE)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    variable->values = calloc(size, sizeof(union Value));
    return variable;
}

void var_freeArrayVariables(struct Interpreter *interpreter)
{
    for (int i = 0; i < interpreter->numArrayVariables; i++)
    {
        struct ArrayVariable *variable = &interpreter->arrayVariables[i];
        if (variable->type == ValueTypeString)
        {
            int numElements = 1;
            for (int di = 0; di < variable->numDimensions; di++)
            {
                numElements *= variable->dimensionSizes[di];
            }
            for (int ei = 0; ei < numElements; ei++)
            {
                union Value *value = &variable->values[ei];
                if (value->stringValue)
                {
                    rcstring_release(value->stringValue);
                }
            }
        }
    }
}
