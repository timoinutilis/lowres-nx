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

#include "cmd_variables.h"
#include "core.h"

enum ErrorCode cmd_LET(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // LET keyword is optional
    if (interpreter->pc->type == TokenLET)
    {
        ++interpreter->pc;
        if (interpreter->pc->type != TokenIdentifier && interpreter->pc->type != TokenStringIdentifier) return ErrorSyntax;
    }
    
    // identifier
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
    if (!varValue) return errorCode;
    if (interpreter->pc->type != TokenEq) return ErrorSyntax;
    ++interpreter->pc;
    
    // value
    struct TypedValue value = itp_evaluateExpression(core, TypeClassAny);
    if (value.type == ValueTypeError) return value.v.errorCode;
    if (value.type != valueType) return ErrorTypeMismatch;
    
    if (interpreter->pass == PassRun)
    {
        if (valueType == ValueTypeString && varValue->stringValue)
        {
            rcstring_release(varValue->stringValue);
        }
        *varValue = value.v;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_DIM(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    bool isGlobal = false;
    struct Token *nextToken = interpreter->pc + 1;
    if (nextToken->type == TokenGLOBAL)
    {
        ++interpreter->pc;
        if (interpreter->pass == PassPrepare && interpreter->subLevel > 0) return ErrorGlobalInsideOfASubprogram;
        isGlobal = true;
    }
    
    do
    {
        // DIM, GLOBAL or comma
        ++interpreter->pc;
        
        // identifier
        struct Token *tokenIdentifier = interpreter->pc;
        ++interpreter->pc;
        if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
        {
            return ErrorSyntax;
        }
        
        int numDimensions = 0;
        int dimensionSizes[MAX_ARRAY_DIMENSIONS];
        
        if (interpreter->pc->type != TokenBracketOpen) return ErrorSyntax;
        ++interpreter->pc;
        
        for (int i = 0; i < MAX_ARRAY_DIMENSIONS; i++)
        {
            struct TypedValue value = itp_evaluateExpression(core, TypeClassNumeric);
            if (value.type == ValueTypeError) return value.v.errorCode;
            
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
        
        if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
        ++interpreter->pc;
        
        if (interpreter->pass == PassRun)
        {
            enum ErrorCode errorCode = ErrorNone;
            struct ArrayVariable *variable = var_dimVariable(interpreter, &errorCode, tokenIdentifier->symbolIndex, numDimensions, dimensionSizes);
            if (!variable) return errorCode;
            variable->type = (tokenIdentifier->type == TokenStringIdentifier) ? ValueTypeString : ValueTypeFloat;
            if (isGlobal)
            {
                variable->subLevel = SUB_LEVEL_GLOBAL;
            }
            interpreter->cycles += variable->numValues;
        }
    }
    while (interpreter->pc->type == TokenComma);
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SWAP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;

    // SWAP
    ++interpreter->pc;
    
    enum ErrorCode errorCode = ErrorNone;
    
    // x identifier
    enum ValueType xValueType = ValueTypeNull;
    union Value *xVarValue = itp_readVariable(core, &xValueType, &errorCode, false);
    if (!xVarValue) return errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // y identifier
    enum ValueType yValueType = ValueTypeNull;
    union Value *yVarValue = itp_readVariable(core, &yValueType, &errorCode, false);
    if (!yVarValue) return errorCode;
    
    if (xValueType != yValueType) return ErrorTypeMismatch;
    
    if (interpreter->pass == PassRun)
    {
        union Value spareValue = *xVarValue;
        *xVarValue = *yVarValue;
        *yVarValue = spareValue;
    }
    
    return itp_endOfCommand(interpreter);
}
