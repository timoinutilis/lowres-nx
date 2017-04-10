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

#include "cmd_maths.h"
#include "core.h"
#include <math.h>
#include <assert.h>
#include <stdlib.h>

struct TypedValue fnc_math1(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // function
    enum TokenType type = interpreter->pc->type;
    interpreter->pc++;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    interpreter->pc++;
    
    // expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    interpreter->pc++;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenSIN:
                value.v.floatValue = sinf(xValue.v.floatValue);
                break;

            case TokenINT:
                value.v.floatValue = floorf(xValue.v.floatValue);
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_math0(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // function
    enum TokenType type = interpreter->pc->type;
    interpreter->pc++;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        switch (type)
        {
            case TokenRND:
                value.v.floatValue = random() / ((float)RAND_MAX + 1.0);
                break;
                
            default:
                assert(0);
                break;
        }
    }
    return value;
}
