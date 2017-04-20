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

#include "cmd_strings.h"
#include "core.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rcstring.h"

struct TypedValue fnc_ASC(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // ASC
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
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

struct TypedValue fnc_BINHEX(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // BIN/HEX
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // optional len expression
    int len = 0;
    int maxLen = (type == TokenBIN) ? 32 : 8;
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;
        
        struct TypedValue lenValue = itp_evaluateNumericExpression(core, 0, maxLen);
        if (lenValue.type == ValueTypeError) return lenValue;
        len = lenValue.v.floatValue;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        int x = xValue.v.floatValue;
        
        struct RCString *rcstring = rcstring_new(NULL, maxLen);
        if (!rcstring) return val_makeError(ErrorOutOfMemory);
        
        if (type == TokenBIN)
        {
            //TODO
        }
        else if (type == TokenHEX)
        {
            if (len > 0)
            {
                snprintf(rcstring->chars, maxLen + 1, "%0*X", len, x);
            }
            else
            {
                snprintf(rcstring->chars, maxLen + 1, "%X", x);
            }
        }
        else
        {
            assert(0);
        }
        resultValue.v.stringValue = rcstring;
    }
    return resultValue;
}

struct TypedValue fnc_CHR(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // CHR$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // expression
    struct TypedValue numericValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numericValue.type == ValueTypeError) return numericValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
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

struct TypedValue fnc_INKEY(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // INKEY$
    ++interpreter->pc;
    
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

struct TypedValue fnc_INSTR(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // INSTR
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // string expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
    ++interpreter->pc;
    
    // search value
    struct TypedValue searchValue = itp_evaluateExpression(core, TypeClassString);
    if (searchValue.type == ValueTypeError) return searchValue;
    
    int startIndex = 0;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // number value
        struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (posValue.type == ValueTypeError) return posValue;
        
        startIndex = posValue.v.floatValue - 1;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        char *string = stringValue.v.stringValue->chars;
        size_t len = strlen(string);
        if (startIndex >= len)
        {
            resultValue.v.floatValue = 0;
        }
        else
        {
            char *found = strstr(&string[startIndex], searchValue.v.stringValue->chars);
            if (found)
            {
                resultValue.v.floatValue = (found - string) + 1;
            }
            else
            {
                resultValue.v.floatValue = 0;
            }
        }
        rcstring_release(stringValue.v.stringValue);
        rcstring_release(searchValue.v.stringValue);
    }
    return resultValue;
}

struct TypedValue fnc_LEFTRIGHT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // LEFT$/RIGHT$
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
    ++interpreter->pc;
    
    struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numberValue.type == ValueTypeError) return numberValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        if (numberValue.v.floatValue < 0) return val_makeError(ErrorInvalidParameter);
        
        size_t len = strlen(stringValue.v.stringValue->chars);
        size_t number = numberValue.v.floatValue;
        
        if (number < len)
        {
            size_t start = (type == TokenLEFT) ? 0 : len - number;
            
            struct RCString *rcstring = rcstring_new(&stringValue.v.stringValue->chars[start], number);
            if (!rcstring) return val_makeError(ErrorOutOfMemory);
            
            resultValue.v.stringValue = rcstring;
        }
        else
        {
            resultValue.v.stringValue = stringValue.v.stringValue;
            rcstring_retain(resultValue.v.stringValue);
        }
        rcstring_release(stringValue.v.stringValue);
    }
    return resultValue;
}

struct TypedValue fnc_LEN(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // LEN
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = strlen(stringValue.v.stringValue->chars);
        rcstring_release(stringValue.v.stringValue);
    }
    return value;
}

struct TypedValue fnc_MID(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // MID$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // string expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
    ++interpreter->pc;
    
    // position value
    struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (posValue.type == ValueTypeError) return posValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
    ++interpreter->pc;
    
    // number value
    struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numberValue.type == ValueTypeError) return numberValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        if (numberValue.v.floatValue < 0) return val_makeError(ErrorInvalidParameter);
        if (posValue.v.floatValue < 1) return val_makeError(ErrorInvalidParameter);
        
        size_t len = strlen(stringValue.v.stringValue->chars);
        size_t index = posValue.v.floatValue - 1;
        size_t number = numberValue.v.floatValue;
        
        if (index >= len)
        {
            resultValue.v.stringValue = interpreter->nullString;
            rcstring_retain(resultValue.v.stringValue);
        }
        else if (index > 0 || number < len)
        {
            if (index + number > len)
            {
                number = len - index;
            }
            struct RCString *rcstring = rcstring_new(&stringValue.v.stringValue->chars[index], number);
            if (!rcstring) return val_makeError(ErrorOutOfMemory);
            
            resultValue.v.stringValue = rcstring;
        }
        else
        {
            resultValue.v.stringValue = stringValue.v.stringValue;
            rcstring_retain(resultValue.v.stringValue);
        }
        
        rcstring_release(stringValue.v.stringValue);
    }
    return resultValue;
}

struct TypedValue fnc_STR(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // STR$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
        ++interpreter->pc;
    
    // expression
    struct TypedValue numericValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numericValue.type == ValueTypeError) return numericValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
        ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        struct RCString *rcstring = rcstring_new(NULL, 20);
        if (!rcstring) return val_makeError(ErrorOutOfMemory);
        
        snprintf(rcstring->chars, 20, "%d", (int)numericValue.v.floatValue); //TODO float values
        resultValue.v.stringValue = rcstring;
    }
    return resultValue;
}

struct TypedValue fnc_VAL(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // VAL
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = atof(stringValue.v.stringValue->chars);
        rcstring_release(stringValue.v.stringValue);
    }
    return value;
}
