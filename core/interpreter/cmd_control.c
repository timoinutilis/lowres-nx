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

#include "cmd_control.h"
#include "lowres_core.h"

enum ErrorCode cmd_END(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    ++interpreter->pc;
    return LRC_endOfCommand(interpreter) ?: ErrorEndOfProgram;
}

enum ErrorCode cmd_IF(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;

    ++interpreter->pc;
    struct TypedValue value = LRC_evaluateExpression(core);
    if (value.type == ValueError) return value.v.errorCode;
    if (value.type != ValueFloat) return ErrorTypeMismatch;
    if (interpreter->pc->type != TokenTHEN) return ErrorExpectedThen;
    ++interpreter->pc;
    if (value.v.floatValue == 0)
    {
        while (   interpreter->pc->type != TokenELSE
               && interpreter->pc->type != TokenEol)
        {
            ++interpreter->pc;
        }
        if (interpreter->pc->type == TokenELSE)
        {
            ++interpreter->pc;
        }
    }
    return ErrorNone;
}

enum ErrorCode cmd_ELSE(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    while (interpreter->pc->type != TokenEol)
    {
        ++interpreter->pc;
    }
    ++interpreter->pc;
    return ErrorNone;
}
