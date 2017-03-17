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

#include "cmd_text.h"
#include "lowres_core.h"

enum ErrorCode cmd_PRINT(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    int newLine = 0;
    ++interpreter->pc;
    do
    {
        struct TypedValue value = LRC_evaluateExpression(core);
        if (value.type == ValueError) return value.v.errorCode;
        
        if (interpreter->pass == PassRun)
        {
            if (value.type == ValueString)
            {
                if (value.v.stringValue)
                {
                    printf("%s", value.v.stringValue->chars);
                }
            }
            else if (value.type == ValueFloat)
            {
                printf("%f", value.v.floatValue);
            }
            else
            {
                printf("<unknown type>");
            }
        }
        
        if (interpreter->pc->type == TokenComma)
        {
            if (interpreter->pass == PassRun)
            {
                printf(" ");
            }
            ++interpreter->pc;
        }
        else if (interpreter->pc->type == TokenSemicolon)
        {
            ++interpreter->pc;
        }
        else
        {
            newLine = 1;
        }
    } while (!LRC_isEndOfCommand(interpreter));
    
    if (interpreter->pass == PassRun && newLine)
    {
        printf("\n");
    }
    return LRC_endOfCommand(interpreter);
}
