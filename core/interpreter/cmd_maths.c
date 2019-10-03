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

#define _USE_MATH_DEFINES
#include "cmd_maths.h"
#include "core.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>

struct TypedValue fnc_math0(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenPI:
                value.v.floatValue = M_PI;
                break;
                
            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math1(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenABS:
                value.v.floatValue = fabsf(xValue.v.floatValue);
                break;
                
            case TokenACOS:
                if (xValue.v.floatValue < -1.0 || xValue.v.floatValue > 1.0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = acosf(xValue.v.floatValue);
                break;
                
            case TokenASIN:
                if (xValue.v.floatValue < -1.0 || xValue.v.floatValue > 1.0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = asinf(xValue.v.floatValue);
                break;
                
            case TokenATAN:
                value.v.floatValue = atanf(xValue.v.floatValue);
                break;
                
            case TokenCOS:
                value.v.floatValue = cosf(xValue.v.floatValue);
                break;
                
            case TokenEXP:
                value.v.floatValue = expf(xValue.v.floatValue);
                break;
                
            case TokenHCOS:
                value.v.floatValue = coshf(xValue.v.floatValue);
                break;
                
            case TokenHSIN:
                value.v.floatValue = sinhf(xValue.v.floatValue);
                break;
                
            case TokenHTAN:
                value.v.floatValue = tanhf(xValue.v.floatValue);
                break;
                
            case TokenINT:
                value.v.floatValue = floorf(xValue.v.floatValue);
                break;
                
            case TokenLOG:
                if (xValue.v.floatValue <= 0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = logf(xValue.v.floatValue);
                break;
                
            case TokenSGN:
                value.v.floatValue = (xValue.v.floatValue > 0) ? 1 : (xValue.v.floatValue < 0) ? BAS_TRUE : BAS_FALSE;
                break;
                
            case TokenSIN:
                value.v.floatValue = sinf(xValue.v.floatValue);
                break;
                
            case TokenSQR:
                if (xValue.v.floatValue < 0) return val_makeError(ErrorInvalidParameter);
                value.v.floatValue = sqrtf(xValue.v.floatValue);
                break;
                
            case TokenTAN:
                value.v.floatValue = tanf(xValue.v.floatValue);
                break;
                                
            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math2(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
        ++interpreter->pc;
    
    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    // y expression
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        float x = xValue.v.floatValue;
        float y = yValue.v.floatValue;
        
        switch (type)
        {
            case TokenMAX:
                value.v.floatValue = (x > y) ? x : y;
                break;
                
            case TokenMIN:
                value.v.floatValue = (x < y) ? x : y;
                break;
                
            default:
                assert(0);
                break;
        }
    }
    return value;
}

enum ErrorCode cmd_RANDOMIZE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // RANDOMIZE
    ++interpreter->pc;
    
    // value
    struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->seed = value.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_RND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // RND
    ++interpreter->pc;
    
    int x = -1;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        ++interpreter->pc;
    
        // expression
        struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (xValue.type == ValueTypeError) return xValue;
        x = xValue.v.floatValue;
        
        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        ++interpreter->pc;
    }
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int seed = (1140671485 * interpreter->seed + 12820163) & 0xFFFFFF;
        interpreter->seed = seed;
        float rnd = seed / (float)0x1000000;
        
        if (x >= 0)
        {
            // integer 0...x
            value.v.floatValue = floorf(rnd * (x + 1));
        }
        else
        {
            // float 0..<1
            value.v.floatValue = rnd;
        }
    }
    return value;
}

enum ErrorCode cmd_ADD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ADD
    ++interpreter->pc;
    
    enum ErrorCode errorCode = ErrorNone;
    
    // Variable
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;
    
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // n vale
    struct TypedValue nValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    bool hasRange = false;
    float base = 0;
    float top = 0;
    
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // base value
        struct TypedValue baseValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (baseValue.type == ValueTypeError) return baseValue.v.errorCode;
        base = baseValue.v.floatValue;
        
        // TO
        if (interpreter->pc->type != TokenTO) return ErrorSyntax;
        ++interpreter->pc;
        
        // top value
        struct TypedValue topValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (topValue.type == ValueTypeError) return topValue.v.errorCode;
        top = topValue.v.floatValue;
        
        hasRange = true;
    }
    
    if (interpreter->pass == PassRun)
    {
        varValue->floatValue += nValue.v.floatValue;
        if (hasRange)
        {
            if (varValue->floatValue < base) varValue->floatValue = top;
            if (varValue->floatValue > top) varValue->floatValue = base;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_INC_DEC(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // INC/DEC
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    enum ErrorCode errorCode = ErrorNone;
    
    // Variable
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, false);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeFloat) return ErrorTypeMismatch;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenINC:
                ++varValue->floatValue;
                break;
                
            case TokenDEC:
                --varValue->floatValue;
                break;
                
            default:
                assert(0);
                break;
        }
    }
    
    return itp_endOfCommand(interpreter);
}
