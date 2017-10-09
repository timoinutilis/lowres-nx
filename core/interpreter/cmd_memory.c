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

#include "cmd_memory.h"
#include "core.h"
#include "data_manager.h"

struct TypedValue fnc_PEEK(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // PEEK
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // expression
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int peek = machine_peek(core, addressValue.v.floatValue);
        if (peek == -1) return val_makeError(ErrorIllegalMemoryAccess);
        resultValue.v.floatValue = peek;
    }
    return resultValue;
}

enum ErrorCode cmd_POKE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // POKE
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;
    
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // poke vale
    struct TypedValue pokeValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (pokeValue.type == ValueTypeError) return pokeValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        bool poke = machine_poke(core, addressValue.v.floatValue, pokeValue.v.floatValue);
        if (!poke) return ErrorIllegalMemoryAccess;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FILL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // FILL
    ++interpreter->pc;

    // start value
    struct TypedValue startValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (startValue.type == ValueTypeError) return startValue.v.errorCode;

    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;

    // length value
    struct TypedValue lengthValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;
    
    int fill = 0;
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;
    
        // fill value
        struct TypedValue fillValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (fillValue.type == ValueTypeError) return fillValue.v.errorCode;
        fill = fillValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        int start = startValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        for (int i = 0; i < length; i++)
        {
            bool poke = machine_poke(core, start + i, fill);
            if (!poke) return ErrorIllegalMemoryAccess;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_COPY(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // COPY
    ++interpreter->pc;
    
    // source value
    struct TypedValue sourceValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (sourceValue.type == ValueTypeError) return sourceValue.v.errorCode;
    
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // length value
    struct TypedValue lengthValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;

    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
    ++interpreter->pc;
    
    // destination value
    struct TypedValue destinationValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (destinationValue.type == ValueTypeError) return destinationValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int source = sourceValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        int destination = destinationValue.v.floatValue;
        for (int i = 0; i < length; i++)
        {
            int peek = machine_peek(core, source + i);
            if (peek == -1) return ErrorIllegalMemoryAccess;
            bool poke = machine_poke(core, destination + i, peek);
            if (!poke) return ErrorIllegalMemoryAccess;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_START_LENGTH(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // START/LENGTH
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // index expression
    struct TypedValue indexValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (indexValue.type == ValueTypeError) return indexValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int index = indexValue.v.floatValue;
        if (type == TokenLENGTH)
        {
            value.v.floatValue = interpreter->romDataManager.entries[index].length;
        }
        else
        {
            value.v.floatValue = interpreter->romDataManager.entries[index].start;
        }
    }
    return value;
}
