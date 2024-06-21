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

#include "cmd_strings.h"
#include "core.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "rcstring.h"

struct TypedValue fnc_ASC(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ASC
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        char ch = stringValue.v.stringValue->chars[0];
        rcstring_release(stringValue.v.stringValue);
        
        if (ch == 0) return val_makeError(ErrorInvalidParameter);
        value.v.floatValue = ch;
    }
    return value;
}

struct TypedValue fnc_BIN_HEX(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // BIN$/HEX$
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // x expression
    struct TypedValue xValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (xValue.type == ValueTypeError) return xValue;
    
    int maxLen = (type == TokenHEX) ? 8 : 16;
    int width = 0;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // width expression
        struct TypedValue widthValue = itp_evaluateNumericExpression(core, 0, maxLen);
        if (widthValue.type == ValueTypeError) return widthValue;
        width = widthValue.v.floatValue;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        int x = xValue.v.floatValue;
        
        struct RCString *rcstring = rcstring_new(NULL, maxLen);
        if (!rcstring) return val_makeError(ErrorOutOfMemory);
        
        if (type == TokenBIN)
        {
            txtlib_itobin(rcstring->chars, maxLen + 1, width, x);
        }
        else if (type == TokenHEX)
        {
#ifdef _MSC_VER
	    _snprintf(rcstring->chars, maxLen, "%0*X", width, x);
	    rcstring->chars[maxLen] = '\0';
#else
            snprintf(rcstring->chars, maxLen + 1, "%0*X", width, x);
#endif
        }
        resultValue.v.stringValue = rcstring;
        interpreter->cycles += maxLen;
    }
    return resultValue;
}

struct TypedValue fnc_CHR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // CHR$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue numericValue = itp_evaluateNumericExpression(core, 0, 255);
    if (numericValue.type == ValueTypeError) return numericValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        char ch = numericValue.v.floatValue;
        struct RCString *rcstring = rcstring_new(&ch, 1);
        if (!rcstring) return val_makeError(ErrorOutOfMemory);
        
        resultValue.v.stringValue = rcstring;
        interpreter->cycles += 1;
    }
    return resultValue;
}

struct TypedValue fnc_INKEY(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // INKEY$
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        if (!core->machine->ioRegisters.attr.keyboardEnabled) return val_makeError(ErrorKeyboardNotEnabled);
        
        char key = core->machine->ioRegisters.key;
        if (key)
        {
            core->machine->ioRegisters.key = 0;
            
            struct RCString *rcstring = rcstring_new(&key, 1);
            if (!rcstring) return val_makeError(ErrorOutOfMemory);
            
            resultValue.v.stringValue = rcstring;
            interpreter->cycles += 1;
        }
        else
        {
            resultValue.v.stringValue = interpreter->nullString;
            rcstring_retain(resultValue.v.stringValue);
        }
    }
    return resultValue;
}

struct TypedValue fnc_INSTR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // INSTR
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // string expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // search value
    struct TypedValue searchValue = itp_evaluateExpression(core, TypeClassString);
    if (searchValue.type == ValueTypeError) return searchValue;
    
    int startIndex = 0;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // number value
        struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (posValue.type == ValueTypeError) return posValue;
        
        startIndex = posValue.v.floatValue - 1;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        char *string = stringValue.v.stringValue->chars;
        char *search = searchValue.v.stringValue->chars;
        size_t stringlen = strlen(string);
        if (startIndex >= stringlen || search[0] == 0)
        {
            resultValue.v.floatValue = 0;
        }
        else
        {
            char *found = strstr(&string[startIndex], search);
            if (found)
            {
                resultValue.v.floatValue = (found - string) + 1;
            }
            else
            {
                resultValue.v.floatValue = 0;
            }
        }
        rcstring_release(stringValue.v.stringValue);
        rcstring_release(searchValue.v.stringValue);
    }
    return resultValue;
}

struct TypedValue fnc_LEFTStr_RIGHTStr(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // LEFT$/RIGHT$
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numberValue.type == ValueTypeError) return numberValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        if (numberValue.v.floatValue < 0) return val_makeError(ErrorInvalidParameter);
        
        size_t len = strlen(stringValue.v.stringValue->chars);
        size_t number = numberValue.v.floatValue;
        
        if (number < len)
        {
            size_t start = (type == TokenLEFTStr) ? 0 : len - number;
            
            struct RCString *rcstring = rcstring_new(&stringValue.v.stringValue->chars[start], number);
            if (!rcstring) return val_makeError(ErrorOutOfMemory);
            
            resultValue.v.stringValue = rcstring;
            interpreter->cycles += number;
        }
        else
        {
            resultValue.v.stringValue = stringValue.v.stringValue;
            rcstring_retain(resultValue.v.stringValue);
        }
        rcstring_release(stringValue.v.stringValue);
    }
    return resultValue;
}

struct TypedValue fnc_LEN(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // LEN
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = strlen(stringValue.v.stringValue->chars);
        rcstring_release(stringValue.v.stringValue);
    }
    return value;
}

struct TypedValue fnc_MID(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // MID$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // string expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // position value
    struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (posValue.type == ValueTypeError) return posValue;

    // comma
    if (interpreter->pc->type != TokenComma) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // number value
    struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numberValue.type == ValueTypeError) return numberValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        if (numberValue.v.floatValue < 0) return val_makeError(ErrorInvalidParameter);
        if (posValue.v.floatValue < 1) return val_makeError(ErrorInvalidParameter);
        
        size_t len = strlen(stringValue.v.stringValue->chars);
        size_t index = posValue.v.floatValue - 1;
        size_t number = numberValue.v.floatValue;
        
        if (index >= len)
        {
            resultValue.v.stringValue = interpreter->nullString;
            rcstring_retain(resultValue.v.stringValue);
        }
        else if (index > 0 || number < len)
        {
            if (index + number > len)
            {
                number = len - index;
            }
            struct RCString *rcstring = rcstring_new(&stringValue.v.stringValue->chars[index], number);
            if (!rcstring) return val_makeError(ErrorOutOfMemory);
            
            resultValue.v.stringValue = rcstring;
            interpreter->cycles += number;
        }
        else
        {
            resultValue.v.stringValue = stringValue.v.stringValue;
            rcstring_retain(resultValue.v.stringValue);
        }
        
        rcstring_release(stringValue.v.stringValue);
    }
    return resultValue;
}

struct TypedValue fnc_STR(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // STR$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue numericValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (numericValue.type == ValueTypeError) return numericValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue resultValue;
    resultValue.type = ValueTypeString;
    
    if (interpreter->pass == PassRun)
    {
        struct RCString *rcstring = rcstring_new(NULL, 20);
        if (!rcstring) return val_makeError(ErrorOutOfMemory);

#ifdef _MSC_VER
	_snprintf(rcstring->chars, 19, "%0.7g", numericValue.v.floatValue);
	rcstring->chars[19] = '\0';
#else
        snprintf(rcstring->chars, 20, "%0.7g", numericValue.v.floatValue);
#endif
        resultValue.v.stringValue = rcstring;
        interpreter->cycles += strlen(rcstring->chars);
    }
    return resultValue;
}

struct TypedValue fnc_VAL(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // VAL
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue stringValue = itp_evaluateExpression(core, TypeClassString);
    if (stringValue.type == ValueTypeError) return stringValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = atof(stringValue.v.stringValue->chars);
        rcstring_release(stringValue.v.stringValue);
    }
    return value;
}

enum ErrorCode cmd_LEFT_RIGHT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // LEFT$/RIGHT$
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return ErrorSyntax;
    ++interpreter->pc;
    
    // variable
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeString) return ErrorTypeMismatch;
    
    size_t number = SIZE_MAX;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // number expression
        struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (numberValue.type == ValueTypeError) return numberValue.v.errorCode;
        number = numberValue.v.floatValue;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
    ++interpreter->pc;
    
    // equal sign
    if (interpreter->pc->type != TokenEq) return ErrorSyntax;
    ++interpreter->pc;
    
    // replace expression
    struct TypedValue replaceValue = itp_evaluateExpression(core, TypeClassString);
    if (replaceValue.type == ValueTypeError) return replaceValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        size_t resultLen = strlen(varValue->stringValue->chars);
        
        struct RCString *resultRCString = varValue->stringValue;
        if (resultRCString->refCount > 1)
        {
            // copy string if shared
            resultRCString = rcstring_new(varValue->stringValue->chars, resultLen);
            rcstring_release(varValue->stringValue);
            varValue->stringValue = resultRCString;
        }
        
        char *resultString = resultRCString->chars;
        char *replaceString = replaceValue.v.stringValue->chars;
        size_t replaceLen = strlen(replaceString);
        if (number > replaceLen)
        {
            number = replaceLen;
        }
        if (number > resultLen)
        {
            number = resultLen;
        }
        
        if (type == TokenLEFTStr)
        {
            for (size_t i = 0; i < number; i++)
            {
                resultString[i] = replaceString[i];
            }
        }
        else if (type == TokenRIGHTStr)
        {
            for (size_t i = 0; i < number; i++)
            {
                resultString[resultLen - 1 - i] = replaceString[replaceLen - 1 - i];
            }
        }
        interpreter->cycles += number;
        
        rcstring_release(replaceValue.v.stringValue);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_MID(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // MID$
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return ErrorSyntax;
    ++interpreter->pc;
    
    // variable
    enum ErrorCode errorCode = ErrorNone;
    enum ValueType valueType = ValueTypeNull;
    union Value *varValue = itp_readVariable(core, &valueType, &errorCode, true);
    if (!varValue) return errorCode;
    if (valueType != ValueTypeString) return ErrorTypeMismatch;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    // position expression
    struct TypedValue posValue = itp_evaluateExpression(core, TypeClassNumeric);
    if (posValue.type == ValueTypeError) return posValue.v.errorCode;
    
    size_t number = SIZE_MAX;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // number expression
        struct TypedValue numberValue = itp_evaluateExpression(core, TypeClassNumeric);
        if (numberValue.type == ValueTypeError) return numberValue.v.errorCode;
        number = numberValue.v.floatValue;
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return ErrorSyntax;
    ++interpreter->pc;
    
    // equal sign
    if (interpreter->pc->type != TokenEq) return ErrorSyntax;
    ++interpreter->pc;
    
    // replace expression
    struct TypedValue replaceValue = itp_evaluateExpression(core, TypeClassString);
    if (replaceValue.type == ValueTypeError) return replaceValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        size_t index = posValue.v.floatValue - 1;
        size_t resultLen = strlen(varValue->stringValue->chars);
        
        struct RCString *resultRCString = varValue->stringValue;
        if (resultRCString->refCount > 1)
        {
            // copy string if shared
            resultRCString = rcstring_new(varValue->stringValue->chars, resultLen);
            rcstring_release(varValue->stringValue);
            varValue->stringValue = resultRCString;
        }
        
        if (index < resultLen)
        {
            char *resultString = resultRCString->chars;
            char *replaceString = replaceValue.v.stringValue->chars;
            size_t replaceLen = strlen(replaceString);
            if (number > replaceLen)
            {
                number = replaceLen;
            }
            if (index + number > resultLen)
            {
                number = resultLen - index;
            }
            
            for (size_t i = 0; i < number; i++)
            {
                resultString[index + i] = replaceString[i];
            }
            interpreter->cycles += number;
        }
        rcstring_release(replaceValue.v.stringValue);
    }
    
    return itp_endOfCommand(interpreter);
}
