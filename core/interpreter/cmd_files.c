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

#include "cmd_files.h"
#include "core.h"
#include <assert.h>
#include <string.h>

enum ErrorCode cmd_LOAD(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;

    // LOAD
    struct Token *startPc = interpreter->pc;
    ++interpreter->pc;
    
    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        bool ready = disk_loadFile(core, fileValue.v.floatValue, addressValue.v.floatValue);
        
        interpreter->exitEvaluation = true;
        if (!ready)
        {
            // disk not ready
            interpreter->pc = startPc;
            interpreter->state = StateWaitForDisk;
            return ErrorNone;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SAVE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // SAVE
    struct Token *startPc = interpreter->pc;
    ++interpreter->pc;
    
    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // comment value
    struct TypedValue commentValue = itp_evaluateExpression(core, TypeClassString);
    if (commentValue.type == ValueTypeError) return commentValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // address value
    struct TypedValue addressValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (addressValue.type == ValueTypeError) return addressValue.v.errorCode;

    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // length value
    struct TypedValue lengthValue = itp_evaluateNumericExpression(core, 1, DATA_SIZE - 1);
    if (lengthValue.type == ValueTypeError) return lengthValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        bool ready = disk_saveFile(core, fileValue.v.floatValue, commentValue.v.stringValue->chars, addressValue.v.floatValue, lengthValue.v.floatValue);
        rcstring_release(commentValue.v.stringValue);
        
        interpreter->exitEvaluation = true;
        if (!ready)
        {
            // disk not ready
            interpreter->pc = startPc;
            interpreter->state = StateWaitForDisk;
            return ErrorNone;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_FILES(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pass == PassRun && interpreter->mode == ModeInterrupt) return ErrorNotAllowedInInterrupt;
    
    // FILES
    struct Token *startPc = interpreter->pc;
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        bool ready = disk_prepare(core);
        
        interpreter->exitEvaluation = true;
        if (!ready)
        {
            // disk not ready
            interpreter->pc = startPc;
            interpreter->state = StateWaitForDisk;
            return ErrorNone;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_FILE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // FILE$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue;

    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;

    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        if (core->diskDrive->dataManager.data == NULL) return val_makeError(ErrorDirectoryNotLoaded);
        
        int index = fileValue.v.floatValue;
        struct DataEntry *entry = &core->diskDrive->dataManager.entries[index];
        
        size_t len = strlen(entry->comment);
        resultValue.v.stringValue = rcstring_new(entry->comment, len);
        rcstring_retain(resultValue.v.stringValue);
        interpreter->cycles += len;
    }
    return resultValue;
}

struct TypedValue fnc_FSIZE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // FSIZE
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // file value
    struct TypedValue fileValue = itp_evaluateNumericExpression(core, 0, MAX_ENTRIES - 1);
    if (fileValue.type == ValueTypeError) return fileValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        if (core->diskDrive->dataManager.data == NULL) return val_makeError(ErrorDirectoryNotLoaded);
        
        int index = fileValue.v.floatValue;
        struct DataEntry *entry = &core->diskDrive->dataManager.entries[index];
        
        resultValue.v.floatValue = entry->length;
    }
    return resultValue;
}
