//
// Copyright 2017-2020 Timo Kloss
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
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

struct TypedValue fnc_UBOUND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // UBOUND
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // array
    if (interpreter->pc->type != TokenIdentifier && interpreter->pc->type != TokenStringIdentifier) return val_makeError(ErrorSyntax);
    int symbolIndex = interpreter->pc->symbolIndex;
    ++interpreter->pc;
    
    int d = 0;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // dimension value
        struct TypedValue dValue = itp_evaluateNumericExpression(core, 1, MAX_ARRAY_DIMENSIONS);
        if (dValue.type == ValueTypeError) return val_makeError(dValue.v.errorCode);
        
        d = dValue.v.floatValue - 1;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        struct ArrayVariable *variable = var_getArrayVariable(interpreter, symbolIndex, interpreter->subLevel);
        if (!variable) return val_makeError(ErrorArrayNotDimensionized);
        
        value.v.floatValue = variable->dimensionSizes[d] - 1;
    }
    return value;
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
