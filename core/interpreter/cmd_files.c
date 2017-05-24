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

#include "cmd_files.h"
#include "core.h"
#include <assert.h>

enum ErrorCode cmd_LOAD(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // LOAD
    ++interpreter->pc;
    
    // file value
    struct TypedValue fileValue = itp_evaluateExpression(core, TypeClassString);
    if (fileValue.type == ValueTypeError) return fileValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        disk_loadFile(core, fileValue.v.stringValue->chars, addressValue.v.floatValue);
        rcstring_release(fileValue.v.stringValue);
        
        interpreter->exitEvaluation = true;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SAVE(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // SAVE
    ++interpreter->pc;
    
    // file value
    struct TypedValue fileValue = itp_evaluateExpression(core, TypeClassString);
    if (fileValue.type == ValueTypeError) return fileValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // length value
    struct TypedValue lengthValue = itp_evaluateNumericExpression(core, 1, DISK_SIZE - 1);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        disk_saveFile(core, fileValue.v.stringValue->chars, addressValue.v.floatValue, lengthValue.v.floatValue);
        rcstring_release(fileValue.v.stringValue);
        
        interpreter->exitEvaluation = true;
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_FIRST(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // FIRST$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
    ++interpreter->pc;
    
    // filter expression
    struct TypedValue filterValue = itp_evaluateExpression(core, TypeClassString);
    if (filterValue.type == ValueTypeError) return filterValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        resultValue.v.stringValue = core->interpreter.nullString;
        rcstring_retain(resultValue.v.stringValue);
        rcstring_release(filterValue.v.stringValue);
    }
    return resultValue;
}

struct TypedValue fnc_NEXT(struct Core *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // NEXT$
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        resultValue.v.stringValue = core->interpreter.nullString;
        rcstring_retain(resultValue.v.stringValue);
    }
    return resultValue;
}
