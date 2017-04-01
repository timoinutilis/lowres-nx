//
// Copyright 2017 Timo Kloss
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

#include "cmd_memory.h"
#include "lowres_core.h"

struct TypedValue fnc_PEEK(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // PEEK
    interpreter->pc++;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return LRC_makeError(ErrorExpectedLeftParenthesis);
    interpreter->pc++;
    
    // expression
    struct TypedValue addressValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueError) return addressValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return LRC_makeError(ErrorExpectedRightParenthesis);
    interpreter->pc++;
    
    struct TypedValue resultValue;
    resultValue.type = ValueFloat;
    
    if (interpreter->pass == PassRun)
    {
        int peek = LRC_peek(&core->machine, addressValue.v.floatValue);
        if (peek == -1) return LRC_makeError(ErrorIllegalMemoryAccess);
        resultValue.v.floatValue = peek;
    }
    return resultValue;
}

enum ErrorCode cmd_POKE(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // POKE
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueError) return addressValue.v.errorCode;
    
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // poke vale
    struct TypedValue pokeValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (pokeValue.type == ValueError) return pokeValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        bool poke = LRC_poke(&core->machine, addressValue.v.floatValue, pokeValue.v.floatValue);
        if (!poke) return ErrorIllegalMemoryAccess;
    }
    
    return LRC_endOfCommand(interpreter);
}

enum ErrorCode cmd_FILL(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // FILL
    ++interpreter->pc;

    // start value
    struct TypedValue startValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (startValue.type == ValueError) return startValue.v.errorCode;

    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;

    // length value
    struct TypedValue lengthValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (lengthValue.type == ValueError) return lengthValue.v.errorCode;
    
    int fill = 0;
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;
    
        // fill value
        struct TypedValue fillValue = LRC_evaluateExpression(core, TypeClassNumeric);
        if (fillValue.type == ValueError) return fillValue.v.errorCode;
        fill = fillValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        int start = startValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        for (int i = 0; i < length; i++)
        {
            bool poke = LRC_poke(&core->machine, start + i, fill);
            if (!poke) return ErrorIllegalMemoryAccess;
        }
    }
    
    return LRC_endOfCommand(interpreter);
}

enum ErrorCode cmd_COPY(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // COPY
    ++interpreter->pc;
    
    // source value
    struct TypedValue sourceValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (sourceValue.type == ValueError) return sourceValue.v.errorCode;
    
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // length value
    struct TypedValue lengthValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (lengthValue.type == ValueError) return lengthValue.v.errorCode;

    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
    ++interpreter->pc;
    
    // destination value
    struct TypedValue destinationValue = LRC_evaluateExpression(core, TypeClassNumeric);
    if (destinationValue.type == ValueError) return destinationValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int source = sourceValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        int destination = destinationValue.v.floatValue;
        for (int i = 0; i < length; i++)
        {
            int peek = LRC_peek(&core->machine, source + i);
            if (peek == -1) return ErrorIllegalMemoryAccess;
            bool poke = LRC_poke(&core->machine, destination + i, peek);
            if (!poke) return ErrorIllegalMemoryAccess;
        }
    }
    
    return LRC_endOfCommand(interpreter);
}
