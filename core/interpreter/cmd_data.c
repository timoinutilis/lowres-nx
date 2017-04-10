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

#include "cmd_data.h"
#include "core.h"

enum ErrorCode cmd_DATA(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
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
        
        if (interpreter->pc->type != TokenFloat && interpreter->pc->type != TokenString) return ErrorUnexpectedToken;
        ++interpreter->pc;
    }
    while (interpreter->pc->type == TokenComma);
    
    // Eol
    if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
    ++interpreter->pc;
    
    return ErrorNone;
}

enum ErrorCode cmd_READ(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    do
    {
         // READ at first, then comma
        ++interpreter->pc;
        
        // variable
        enum ValueType varType = ValueTypeNull;
        enum ErrorCode errorCode = ErrorNone;
        union Value *varValue = itp_readVariable(core, &varType, &errorCode);
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
    struct Interpreter *interpreter = &core->interpreter;
    
    // RESTORE
    struct Token *tokenRESTORE = interpreter->pc;
    ++interpreter->pc;
    
    // optional jump label
    if (interpreter->pc->type == TokenIdentifier)
    {
        if (interpreter->pass == PassPrepare)
        {
            struct JumpLabelItem *item = lab_getJumpLabel(interpreter, interpreter->pc->symbolIndex);
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
