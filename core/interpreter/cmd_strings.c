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

#include "cmd_strings.h"
#include "core.h"
#include <string.h>
#include <stdio.h>
#include "rcstring.h"

struct TypedValue fnc_STR(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // STR$
    interpreter->pc++;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    interpreter->pc++;
    
    // expression
    struct TypedValue numericValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numericValue.type == ValueTypeError) return numericValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    interpreter->pc++;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        struct RCString *rcstring = rcstring_new(NULL, 20);
        if (!rcstring) return val_makeError(ErrorOutOfMemory);
        
        snprintf(rcstring->chars, 20, "%d", (int)numericValue.v.floatValue);
        resultValue.v.stringValue = rcstring;
    }
    return resultValue;
}

struct TypedValue fnc_ASC(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // ASC
    interpreter->pc++;

    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    interpreter->pc++;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    interpreter->pc++;

    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        char ch = stringValue.v.stringValue->chars[0];
        rcstring_release(stringValue.v.stringValue);
        
        if (ch == 0) return val_makeError(ErrorInvalidParameter);
        value.v.floatValue = ch;
    }
    return value;
}

struct TypedValue fnc_CHR(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // CHR$
    interpreter->pc++;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    interpreter->pc++;
    
    // expression
    struct TypedValue numericValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numericValue.type == ValueTypeError) return numericValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    interpreter->pc++;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        if (numericValue.v.floatValue < 0 || numericValue.v.floatValue > 255) return val_makeError(ErrorInvalidParameter);
        
        char ch = numericValue.v.floatValue;
        struct RCString *rcstring = rcstring_new(&ch, 1);
        if (!rcstring) return val_makeError(ErrorOutOfMemory);
        
        resultValue.v.stringValue = rcstring;
    }
    return resultValue;
}

struct TypedValue fnc_LEN(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // LEN
    interpreter->pc++;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    interpreter->pc++;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    interpreter->pc++;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = strlen(stringValue.v.stringValue->chars);
        rcstring_release(stringValue.v.stringValue);
    }
    return value;
}

struct TypedValue fnc_INKEY(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // INKEY$
    interpreter->pc++;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        char key = core->machine.ioRegisters.key;
        if (key)
        {
            core->machine.ioRegisters.key = 0;
            
            struct RCString *rcstring = rcstring_new(&key, 1);
            if (!rcstring) return val_makeError(ErrorOutOfMemory);
            
            resultValue.v.stringValue = rcstring;
        }
        else
        {
            resultValue.v.stringValue = interpreter->nullString;
            rcstring_retain(resultValue.v.stringValue);
        }
    }
    return resultValue;
}
