//
// Copyright 2017 Timo Kloss
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

#include "cmd_data.h"
#include "core.h"

enum ErrorCode cmd_DATA(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    if (interpreter->pass == PassPrepare)
    {
        if (!interpreter->firstData)
        {
            interpreter->firstData = interpreter->pc;
        }
        if (interpreter->lastData)
        {
            interpreter->lastData->jumpToken = interpreter->pc;
        }
        interpreter->lastData = interpreter->pc;
    }
    
    do
    {
        ++interpreter->pc; // DATA at first, then comma
        
        if (interpreter->pc->type == TokenString)
        {
            ++interpreter->pc;
        }
        else if (interpreter->pc->type == TokenFloat)
        {
            ++interpreter->pc;
        }
        else if (interpreter->pc->type == TokenMinus)
        {
            ++interpreter->pc;
            if (interpreter->pc->type != TokenFloat) return ErrorSyntax;
            ++interpreter->pc;
        }
        else
        {
            return ErrorSyntax;
        }
    }
    while (interpreter->pc->type == TokenComma);
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorSyntax;
    ++interpreter->pc;
    
    return ErrorNone;
}

enum ErrorCode cmd_READ(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    do
    {
         // READ at first, then comma
        ++interpreter->pc;
        
        // variable
        enum ValueType varType = ValueTypeNull;
        enum ErrorCode errorCode = ErrorNone;
        union Value *varValue = itp_readVariable(core, &varType, &errorCode, true);
        if (!varValue) return errorCode;
            
        if (interpreter->pass == PassRun)
        {
            if (!interpreter->currentDataValueToken) return ErrorOutOfData;
            
            struct Token *dataValueToken = interpreter->currentDataValueToken;
            if (dataValueToken->type == TokenFloat)
            {
                if (varType != ValueTypeFloat) return ErrorTypeMismatch;
                varValue->floatValue = dataValueToken->floatValue;
            }
            else if (dataValueToken->type == TokenMinus)
            {
                if (varType != ValueTypeFloat) return ErrorTypeMismatch;
                interpreter->currentDataValueToken++;
                varValue->floatValue = -interpreter->currentDataValueToken->floatValue;
            }
            else if (dataValueToken->type == TokenString)
            {
                if (varType != ValueTypeString) return ErrorTypeMismatch;
                if (varValue->stringValue)
                {
                    rcstring_release(varValue->stringValue);
                }
                varValue->stringValue = dataValueToken->stringValue;
                rcstring_retain(varValue->stringValue);
            }
            
            dat_nextData(interpreter);
        }
    }
    while (interpreter->pc->type == TokenComma);
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_RESTORE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // RESTORE
    struct Token *tokenRESTORE = interpreter->pc;
    ++interpreter->pc;
    
    // optional jump label
    if (interpreter->pc->type == TokenIdentifier)
    {
        if (interpreter->pass == PassPrepare)
        {
            struct JumpLabelItem *item = tok_getJumpLabel(&interpreter->tokenizer, interpreter->pc->symbolIndex);
            if (!item) return ErrorUndefinedLabel;
            tokenRESTORE->jumpToken = item->token;
        }
        else if (interpreter->pass == PassRun)
        {
            // find DATA after label
            dat_restoreData(interpreter, tokenRESTORE->jumpToken);
        }
        ++interpreter->pc;
    }
    else if (interpreter->pass == PassRun)
    {
        // restore to first DATA
        dat_restoreData(interpreter, NULL);
    }
    
    return itp_endOfCommand(interpreter);
}
