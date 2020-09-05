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

#include "variables.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>

struct SimpleVariable *var_getSimpleVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel)
{
    struct SimpleVariable *variable = NULL;
    for (int i = interpreter->numSimpleVariables - 1; i >= 0; i--)
    {
        variable = &interpreter->simpleVariables[i];
        if (variable->symbolIndex == symbolIndex && (variable->subLevel == subLevel || variable->subLevel == SUB_LEVEL_GLOBAL))
        {
            // variable found
            return variable;
        }
    }
    return NULL;
}

struct SimpleVariable *var_createSimpleVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, enum ValueType type, union Value *valueReference)
{
    if (interpreter->numSimpleVariables >= MAX_SIMPLE_VARIABLES)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    if (subLevel > 127)
    {
        *errorCode = ErrorStackOverflow;
        return NULL;
    }
    struct SimpleVariable *variable = &interpreter->simpleVariables[interpreter->numSimpleVariables];
    interpreter->numSimpleVariables++;
    memset(variable, 0, sizeof(struct SimpleVariable));
    variable->symbolIndex = symbolIndex;
    variable->subLevel = subLevel;
    variable->type = type;
    if (valueReference)
    {
        variable->isReference = 1;
        variable->v.reference = valueReference;
    }
    else
    {
        variable->isReference = 0;
        if (type == ValueTypeString)
        {
            // assign global NullString
            variable->v.stringValue = interpreter->nullString;
            rcstring_retain(variable->v.stringValue);
        }
    }
    return variable;
}

void var_freeSimpleVariables(struct Interpreter *interpreter, int minSubLevel)
{
    for (int i = interpreter->numSimpleVariables - 1; i >= 0; i--)
    {
        struct SimpleVariable *variable = &interpreter->simpleVariables[i];
        if (variable->subLevel < minSubLevel)
        {
            break;
        }
        else
        {
            if (!variable->isReference && variable->type == ValueTypeString)
            {
                rcstring_release(variable->v.stringValue);
            }
            interpreter->numSimpleVariables--;
        }
    }
}

struct ArrayVariable *var_getArrayVariable(struct Interpreter *interpreter, int symbolIndex, int subLevel)
{
    struct ArrayVariable *variable = NULL;
    for (int i = interpreter->numArrayVariables - 1; i >= 0; i--)
    {
        variable = &interpreter->arrayVariables[i];
        if (variable->symbolIndex == symbolIndex && (variable->subLevel == subLevel || variable->subLevel == SUB_LEVEL_GLOBAL))
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
    if (var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel))
    {
        *errorCode = ErrorArrayAlreadyDimensionized;
        return NULL;
    }
    if (var_getSimpleVariable(interpreter, symbolIndex, interpreter->subLevel))
    {
        *errorCode = ErrorVariableAlreadyUsed;
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
    variable->subLevel = interpreter->subLevel;
    variable->isReference = 0;
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
    if (!variable->values) exit(EXIT_FAILURE);
    
    variable->numValues = (int)size;
    return variable;
}

struct ArrayVariable *var_createArrayVariable(struct Interpreter *interpreter, enum ErrorCode *errorCode, int symbolIndex, int subLevel, struct ArrayVariable *arrayReference)
{
    if (interpreter->numArrayVariables >= MAX_ARRAY_VARIABLES)
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    if (subLevel > 127)
    {
        *errorCode = ErrorStackOverflow;
        return NULL;
    }
    struct ArrayVariable *variable = &interpreter->arrayVariables[interpreter->numArrayVariables];
    interpreter->numArrayVariables++;
    memset(variable, 0, sizeof(struct ArrayVariable));
    variable->symbolIndex = symbolIndex;
    variable->subLevel = subLevel;
    variable->isReference = 1;
    variable->type = arrayReference->type;
    int numDimensions = arrayReference->numDimensions;
    variable->numDimensions = numDimensions;
    for (int i = 0; i < numDimensions; i++)
    {
        variable->dimensionSizes[i] = arrayReference->dimensionSizes[i];
    }
    variable->values = arrayReference->values;
    return variable;
}

void var_freeArrayVariables(struct Interpreter *interpreter, int minSubLevel)
{
    for (int i = interpreter->numArrayVariables - 1; i >= 0; i--)
    {
        struct ArrayVariable *variable = &interpreter->arrayVariables[i];
        if (variable->subLevel < minSubLevel)
        {
            break;
        }
        else
        {
            if (!variable->isReference)
            {
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
                free(variable->values);
                variable->values = NULL;
            }
            interpreter->numArrayVariables--;
        }
    }
}
