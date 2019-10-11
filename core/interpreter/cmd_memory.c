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
#include <assert.h>

struct TypedValue fnc_PEEK(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // PEEK/W/L
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenPEEK:
            {
                int peek = machine_peek(core, addressValue.v.floatValue);
                if (peek == -1) return val_makeError(ErrorIllegalMemoryAccess);
                resultValue.v.floatValue = peek;
                break;
            }
            
            case TokenPEEKW:
            {
                int peek1 = machine_peek(core, addressValue.v.floatValue);
                int peek2 = machine_peek(core, addressValue.v.floatValue + 1);
                if (peek1 == -1 || peek2 == -1) return val_makeError(ErrorIllegalMemoryAccess);
                
                int16_t value = peek1 | (peek2 << 8);
                resultValue.v.floatValue = value;
                break;
            }
            
            case TokenPEEKL:
            {
                int peek1 = machine_peek(core, addressValue.v.floatValue);
                int peek2 = machine_peek(core, addressValue.v.floatValue + 1);
                int peek3 = machine_peek(core, addressValue.v.floatValue + 2);
                int peek4 = machine_peek(core, addressValue.v.floatValue + 3);
                if (peek1 == -1 || peek2 == -1 || peek3 == -1 || peek4 == -1) return val_makeError(ErrorIllegalMemoryAccess);
                
                int32_t value = peek1 | (peek2 << 8) | (peek3 << 16) | (peek4 << 24);
                resultValue.v.floatValue = value;
                break;
            }
            
            default:
                assert(0);
        }
    }
    return resultValue;
}

enum ErrorCode cmd_POKE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // POKE/W/L
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;
    
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // poke vale
    struct TypedValue pokeValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (pokeValue.type == ValueTypeError) return pokeValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenPOKE:
            {
                bool poke = machine_poke(core, addressValue.v.floatValue, pokeValue.v.floatValue);
                if (!poke) return ErrorIllegalMemoryAccess;
                break;
            }
            
            case TokenPOKEW:
            {
                int16_t value = pokeValue.v.floatValue;
                bool poke1 = machine_poke(core, addressValue.v.floatValue    , value);
                bool poke2 = machine_poke(core, addressValue.v.floatValue + 1, value >> 8);
                if (!poke1 || !poke2) return ErrorIllegalMemoryAccess;
                break;
            }
            
            case TokenPOKEL:
            {
                int32_t value = pokeValue.v.floatValue;
                bool poke1 = machine_poke(core, addressValue.v.floatValue    , value);
                bool poke2 = machine_poke(core, addressValue.v.floatValue + 1, value >> 8);
                bool poke3 = machine_poke(core, addressValue.v.floatValue + 2, value >> 16);
                bool poke4 = machine_poke(core, addressValue.v.floatValue + 3, value >> 24);
                if (!poke1 || !poke2 || !poke3 || !poke4) return ErrorIllegalMemoryAccess;
                break;
            }
                
            default:
                assert(0);
        }

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

    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
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
        interpreter->cycles += length;
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
    
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // length value
    struct TypedValue lengthValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;

    if (interpreter->pc->type != TokenTO) return ErrorSyntax;
    ++interpreter->pc;
    
    // destination value
    struct TypedValue destinationValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (destinationValue.type == ValueTypeError) return destinationValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int source = sourceValue.v.floatValue;
        int length = lengthValue.v.floatValue;
        int destination = destinationValue.v.floatValue;
        if (source < destination)
        {
            for (int i = length - 1; i >= 0; i--)
            {
                int peek = machine_peek(core, source + i);
                if (peek == -1) return ErrorIllegalMemoryAccess;
                bool poke = machine_poke(core, destination + i, peek);
                if (!poke) return ErrorIllegalMemoryAccess;
            }
        }
        else if (source > destination)
        {
            for (int i = 0; i < length; i++)
            {
                int peek = machine_peek(core, source + i);
                if (peek == -1) return ErrorIllegalMemoryAccess;
                bool poke = machine_poke(core, destination + i, peek);
                if (!poke) return ErrorIllegalMemoryAccess;
            }
        }
        interpreter->cycles += length;
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_ROM_SIZE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ROM/SIZE
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // index expression
    struct TypedValue indexValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (indexValue.type == ValueTypeError) return indexValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int index = indexValue.v.floatValue;
        if (type == TokenSIZE)
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

enum ErrorCode cmd_ROL_ROR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ROL/ROR
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;
    
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // n vale
    struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int value = machine_peek(core, addressValue.v.floatValue);
        if (value == -1) return ErrorIllegalMemoryAccess;
        
        int n = (int)nValue.v.floatValue;
        if (type == TokenROR)
        {
            n = -n;
        }
        n &= 0x07;
        
        value = value << n;
        value = value | (value >> 8);
        
        bool poke = machine_poke(core, addressValue.v.floatValue, value);
        if (!poke) return ErrorIllegalMemoryAccess;
    }
    
    return itp_endOfCommand(interpreter);
}
