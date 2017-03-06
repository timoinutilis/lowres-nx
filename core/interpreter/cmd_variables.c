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
    
    if (interpreter->pc->type == TokenLET)
    {
        ++interpreter->pc;
        if (interpreter->pc->type != TokenIdentifier && interpreter->pc->type != TokenStringIdentifier) return ErrorExpectedVariableIdentifier;
    }
    enum ErrorCode errorCode = ErrorNone;
    union Value *varValue = LRC_readVariable(core, &errorCode);
    if (!varValue) return errorCode;
    if (interpreter->pc->type != TokenEq) return ErrorExpectedEqualSign;
    ++interpreter->pc;
    struct TypedValue value = LRC_evaluateExpression(core);
    if (value.type == ValueError) return value.v.errorCode;
    varValue->floatValue = value.v.floatValue;
    return LRC_endOfCommand(interpreter);
}
