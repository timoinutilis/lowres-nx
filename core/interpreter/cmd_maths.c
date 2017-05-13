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

#include "cmd_maths.h"
#include "core.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>

struct TypedValue fnc_math0(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
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
                
            case TokenRND:
                value.v.floatValue = random() / ((float)RAND_MAX + 1.0);
                break;
                
            case TokenTIMER:
                value.v.floatValue = 0; //TODO
                break;
                
            case TokenRASTER:
                value.v.floatValue = core->machine.videoRegisters.rasterLine;
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
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
                
            case TokenATN:
                value.v.floatValue = atanf(xValue.v.floatValue);
                break;
                
            case TokenCOS:
                value.v.floatValue = cosf(xValue.v.floatValue);
                break;
                
            case TokenEXP:
                value.v.floatValue = expf(xValue.v.floatValue);
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // function
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
        ++interpreter->pc;
    
    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
    ++interpreter->pc;

    // y expression
    struct TypedValue yValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (yValue.type == ValueTypeError) return yValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // RANDOMIZE
    ++interpreter->pc;
    
    // value
    struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
    if (value.type == ValueTypeError) return value.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        srandom(value.v.floatValue);
    }
    
    return itp_endOfCommand(interpreter);
}
