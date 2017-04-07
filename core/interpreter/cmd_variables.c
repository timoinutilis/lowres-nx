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

#include "cmd_variables.h"
#include "lowres_core.h"

enum ErrorCode cmd_LET(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // LET keyword is optional
    if (interpreter->pc->type == TokenLET)
    {
        ++interpreter->pc;
        if (interpreter->pc->type != TokenIdentifier && interpreter->pc->type != TokenStringIdentifier) return ErrorExpectedVariableIdentifier;
    }
    
    // identifier
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueNull;
    union Value *varValue = LRC_readVariable(core, &valueType, &errorCode);
    if (!varValue) return errorCode;
    if (interpreter->pc->type != TokenEq) return ErrorExpectedEqualSign;
    ++interpreter->pc;
    
    // value
    struct TypedValue value = LRC_evaluateExpression(core, TypeClassAny);
    if (value.type == ValueError) return value.v.errorCode;
    if (value.type != valueType) return ErrorTypeMismatch;
    
    if (interpreter->pass == PassRun)
    {
        if (valueType == ValueString && varValue->stringValue)
        {
            rcstring_release(varValue->stringValue);
        }
        *varValue = value.v;
    }
    
    return LRC_endOfCommand(interpreter);
}

enum ErrorCode cmd_DIM(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt)
    {
        return ErrorNotAllowedInInterrupt;
    }
    
    // DIM
    ++interpreter->pc;
    
    // identifier
    struct Token *tokenIdentifier = interpreter->pc;
    ++interpreter->pc;
    if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
    {
        return ErrorExpectedVariableIdentifier;
    }
    
    int numDimensions = 0;
    int dimensionSizes[MAX_ARRAY_DIMENSIONS];
    
    if (interpreter->pc->type != TokenBracketOpen) return ErrorExpectedLeftParenthesis;
    ++interpreter->pc;
    
    for (int i = 0; i < MAX_ARRAY_DIMENSIONS; i++)
    {
        struct TypedValue value = LRC_evaluateExpression(core, TypeClassNumeric);
        if (value.type == ValueError) return value.v.errorCode;
        
        dimensionSizes[i] = value.v.floatValue + 1; // value is max index, so size is +1
        numDimensions++;
        
        if (interpreter->pc->type == TokenComma)
        {
            ++interpreter->pc;
        }
        else
        {
            break;
        }
    }
    
    if (interpreter->pc->type != TokenBracketClose) return ErrorExpectedRightParenthesis;
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        enum ErrorCode errorCode = ErrorNone;
        struct ArrayVariable *variable = LRC_dimVariable(interpreter, &errorCode, tokenIdentifier->symbolIndex, numDimensions, dimensionSizes);
        if (!variable) return errorCode;
        variable->type = (tokenIdentifier->type == TokenStringIdentifier) ? ValueString : ValueFloat;
    }
    
    return LRC_endOfCommand(interpreter);
}
