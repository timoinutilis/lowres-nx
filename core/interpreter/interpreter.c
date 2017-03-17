//
// Copyright 2016-2017 Timo Kloss
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

#include "interpreter.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include "lowres_core.h"
#include "cmd_control.h"
#include "cmd_variables.h"
#include "cmd_text.h"

enum ErrorCode LRC_tokenizeProgram(struct LowResCore *core, const char *sourceCode);
struct TypedValue LRC_evaluateExpressionLevel(struct LowResCore *core, int level);
struct TypedValue LRC_evaluatePrimaryExpression(struct LowResCore *core);
enum ErrorCode LRC_evaluateCommand(struct LowResCore *core);

enum ErrorCode LRC_compileProgram(struct LowResCore *core, const char *sourceCode)
{
    // Tokenize
    
    enum ErrorCode errorCode = LRC_tokenizeProgram(core, sourceCode);
    if (errorCode != ErrorNone) return errorCode;
    
    struct Interpreter *interpreter = &core->interpreter;
    
    // Prepare
    
    interpreter->pc = interpreter->tokens;
    interpreter->pass = PassPrepare;
    do
    {
        errorCode = LRC_evaluateCommand(core);
    } while (errorCode == ErrorNone);
    
    if (errorCode != ErrorNone && errorCode != ErrorEndOfProgram) return errorCode;
    assert(interpreter->numLabelStackItems == 0);
    
    return ErrorNone;
}

enum ErrorCode LRC_runProgram(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    interpreter->pc = interpreter->tokens;
    interpreter->pass = PassRun;
    enum ErrorCode errorCode = ErrorNone;
    
    do
    {
        errorCode = LRC_evaluateCommand(core);
    } while (errorCode == ErrorNone);
    
    return errorCode;
}

void LRC_freeProgram(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    
    // Free variables
    for (int i = 0; i < interpreter->numSimpleVariables; i++)
    {
        struct SimpleVariable *variable = &interpreter->simpleVariables[i];
        if (variable->type == ValueString)
        {
            rcstring_release(variable->v.stringValue);
        }
    }
    
    // Free string tokens
    for (int i = 0; i < interpreter->numTokens; i++)
    {
        struct Token *token = &interpreter->tokens[i];
        if (token->type == TokenString)
        {
            assert(token->stringValue && token->stringValue->refCount == 1);
            rcstring_release(token->stringValue);
        }
    }
}

enum ErrorCode LRC_tokenizeProgram(struct LowResCore *core, const char *sourceCode)
{
    const char *charSetDigits = "0123456789";
    const char *charSetLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    const char *charSetAlphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
    
    struct Interpreter *interpreter = &core->interpreter;
    const char *character = sourceCode;
    
//    uint8_t *currentRomByte = core->machine.cartridgeRom;
    
    while (*character)
    {
        if (interpreter->numTokens >= MAX_TOKENS)
        {
            return ErrorTooManyTokens;
        }
        struct Token *token = &interpreter->tokens[interpreter->numTokens];
        
        // line break
        if (*character == '\n')
        {
            token->type = TokenEol;
            interpreter->numTokens++;
            character++;
            continue;
        }
        
        // space
        if (*character == ' ' || *character == '\t')
        {
            character++;
            continue;
        }
        
        // string
        if (*character == '"')
        {
            character++;
            const char *firstCharacter = character;
            while (*character && *character != '"')
            {
                character++;
                if (*character == '\n')
                {
                    return ErrorExpectedEndOfString;
                }
            }
            int len = (int)(character - firstCharacter);
            struct RCString *string = rcstring_new(firstCharacter, len);
            if (!string) return ErrorOutOfMemory;
            token->type = TokenString;
            token->stringValue = string;
            interpreter->numTokens++;
            character++;
            continue;
        }
        
        // number
        if (strchr(charSetDigits, *character))
        {
            float number = 0;
            int afterDot = 0;
            while (*character)
            {
                if (strchr(charSetDigits, *character))
                {
                    int digit = (int)*character - (int)'0';
                    if (afterDot == 0)
                    {
                        number *= 10;
                        number += digit;
                    }
                    else
                    {
                        number += (float)digit / afterDot;
                        afterDot *= 10;
                    }
                    character++;
                }
                else if (*character == '.' && afterDot == 0)
                {
                    afterDot = 10;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            interpreter->numTokens++;
            continue;
        }
        
        // Keyword
        enum TokenType foundKeywordToken = TokenUndefined;
        for (int i = 0; i < Token_count; i++)
        {
            const char *keyword = TokenStrings[i];
            if (keyword)
            {
                size_t keywordLen = strlen(keyword);
                int keywordIsAlphaNum = strchr(charSetAlphaNum, keyword[0]) != NULL;
                for (int pos = 0; pos <= keywordLen && character[pos]; pos++)
                {
                    char textCharacter = character[pos];
                    
                    if (pos < keywordLen)
                    {
                        char symbCharacter = keyword[pos];
                        if (symbCharacter != textCharacter)
                        {
                            // not matching
                            break;
                        }
                    }
                    else if (keywordIsAlphaNum && strchr(charSetAlphaNum, textCharacter))
                    {
                        // matching, but word is longer, so seems to be an identifier
                        break;
                    }
                    else
                    {
                        // symbol found!
                        foundKeywordToken = i;
                        character += keywordLen;
                        break;
                    }
                }
                if (foundKeywordToken != TokenUndefined)
                {
                    break;
                }
            }
        }
        if (foundKeywordToken != TokenUndefined)
        {
            if (foundKeywordToken == TokenREM)
            {
                // REM comment, skip until end of line
                while (*character)
                {
                    character++;
                    if (*character == '\n')
                    {
                        character++;
                        break;
                    }
                }
            }
            else
            {
                token->type = foundKeywordToken;
                interpreter->numTokens++;
            }
            continue;
        }
        
        // Symbol
        if (strchr(charSetLetters, *character))
        {
            const char *firstCharacter = character;
            char isString = 0;
            while (*character)
            {
                if (strchr(charSetAlphaNum, *character))
                {
                    character++;
                }
                else
                {
                    if (*character == '$')
                    {
                        isString = 1;
                        character++;
                    }
                    break;
                }
            }
            if (interpreter->numSymbols >= MAX_SYMBOLS)
            {
                return ErrorTooManySymbols;
            }
            int len = (int)(character - firstCharacter);
            if (len >= SYMBOL_NAME_SIZE)
            {
                return ErrorSymbolNameTooLong;
            }
            char symbolName[SYMBOL_NAME_SIZE];
            memcpy(symbolName, firstCharacter, len);
            symbolName[len] = 0;
            int symbolIndex = -1;
            // find existing symbol
            for (int i = 0; i < MAX_SYMBOLS && interpreter->symbols[i].name[0] != 0; i++)
            {
                if (strcmp(symbolName, interpreter->symbols[i].name) == 0)
                {
                    symbolIndex = i;
                    break;
                }
            }
            if (symbolIndex == -1)
            {
                // add new symbol
                strcpy(interpreter->symbols[interpreter->numSymbols].name, symbolName);
                symbolIndex = interpreter->numSymbols++;
            }
            if (isString)
            {
                token->type = TokenStringIdentifier;
            }
            else if (*character == ':')
            {
                token->type = TokenLabel;
                character++;
                enum ErrorCode errorCode = LRC_setJumpLabel(interpreter, symbolIndex, token + 1);
                if (errorCode != ErrorNone) return errorCode;
            }
            else
            {
                token->type = TokenIdentifier;
            }
            token->symbolIndex = symbolIndex;
            interpreter->numTokens++;
            continue;
        }
        
        // Unexpected character
        return ErrorUnexpectedCharacter;
    }
    return ErrorNone;
}

union Value *LRC_readVariable(struct LowResCore *core, enum ValueType *type, enum ErrorCode *errorCode)
{
    struct Interpreter *interpreter = &core->interpreter;

    struct Token *tokenIdentifier = interpreter->pc;
    
    if (tokenIdentifier->type != TokenIdentifier && tokenIdentifier->type != TokenStringIdentifier)
    {
        *errorCode = ErrorExpectedVariableIdentifier;
        return NULL;
    }
    
    if (type)
    {
        if (tokenIdentifier->type == TokenIdentifier)
        {
            *type = ValueFloat;
        }
        else if (tokenIdentifier->type == TokenStringIdentifier)
        {
            *type = ValueString;
        }
    }
    
    int symbolIndex = tokenIdentifier->symbolIndex;
    ++interpreter->pc;
    
    if (interpreter->pass == PassRun)
    {
        struct SimpleVariable *variable = NULL;
        for (int i = 0; i < interpreter->numSimpleVariables; i++)
        {
            variable = &interpreter->simpleVariables[i];
            if (variable->symbolIndex == symbolIndex)
            {
                // variable found
                return &variable->v;
            }
        }
        
        // create new variable
        if (interpreter->numSimpleVariables >= MAX_SIMPLE_VARIABLES)
        {
            *errorCode = ErrorOutOfMemory;
            return NULL;
        }
        variable = &interpreter->simpleVariables[interpreter->numSimpleVariables];
        interpreter->numSimpleVariables++;
        memset(variable, 0, sizeof(struct SimpleVariable));
        variable->symbolIndex = symbolIndex;
        variable->type = (tokenIdentifier->type == TokenStringIdentifier) ? ValueString : ValueFloat;
        return &variable->v;
    }
    else
    {
        return &ValueDummy;
    }
    
    /*
    if (interpreter->pc->type == TokenBracketOpen)
    {
        ++interpreter->pc;
        struct TypedValue indexValue = LRC_evaluateExpression(core);
        if (indexValue.type == TypeError)
        {
            *errorCode = indexValue.v.errorCode;
            return NULL;
        }

        if (interpreter->pc->type != TokenBracketClose)
        {
            *errorCode = ErrorExpectedRightParenthesis;
            return NULL;
        }
        ++interpreter->pc;
    }
     */
}

struct TypedValue LRC_evaluateExpression(struct LowResCore *core)
{
    return LRC_evaluateExpressionLevel(core, 0);
}

int LRC_isTokenLevel(enum TokenType token, int level)
{
    switch (level)
    {
        case 0:
            return token == TokenXOR || token == TokenOR;
        case 1:
            return token == TokenAND;
//        case 2:
//            return token == TokenNOT;
        case 3:
            return token == TokenEq || token == TokenUneq || token == TokenGr || token == TokenLe || token == TokenGrEq || token == TokenLeEq;
        case 4:
            return token == TokenPlus || token == TokenMinus;
        case 5:
            return token == TokenMOD;
        case 6:
            return token == TokenMul || token == TokenDiv;
//        case 7:
//            return token == TokenPlus || token == TokenMinus; // unary
        case 8:
            return token == TokenPow;
    }
    return 0;
}

struct TypedValue LRC_evaluateExpressionLevel(struct LowResCore *core, int level)
{
    struct Interpreter *interpreter = &core->interpreter;
    enum TokenType type = interpreter->pc->type;
    
    if (level == 2 && type == TokenNOT)
    {
        ++interpreter->pc;
        struct TypedValue value = LRC_evaluateExpressionLevel(core, level + 1);
        if (value.type == ValueError) return value;
        value.v.floatValue = ~((int)value.v.floatValue);
        return value;
    }
    if (level == 7 && (type == TokenPlus || type == TokenMinus))
    {
        ++interpreter->pc;
        struct TypedValue value = LRC_evaluateExpressionLevel(core, level + 1);
        if (value.type == ValueError) return value;
        if (type == TokenMinus)
        {
            value.v.floatValue = -value.v.floatValue;
        }
        return value;
    }
    if (level == 9)
    {
        return LRC_evaluatePrimaryExpression(core);
    }
    
    struct TypedValue value = LRC_evaluateExpressionLevel(core, level + 1);
    if (value.type == ValueError) return value;
    
    while (LRC_isTokenLevel(interpreter->pc->type, level))
    {
        enum TokenType type = interpreter->pc->type;
        ++interpreter->pc;
        struct TypedValue rightValue = LRC_evaluateExpressionLevel(core, level + 1);
        if (rightValue.type == ValueError) return rightValue;
        
        struct TypedValue newValue;
        if (value.type != rightValue.type)
        {
            newValue.type = ValueError;
            newValue.v.errorCode = ErrorTypeMismatch;
            return newValue;
        }
        newValue.type = value.type;
        
        switch (type)
        {
            case TokenXOR: {
                int leftInt = value.v.floatValue;
                int rightInt = rightValue.v.floatValue;
                newValue.v.floatValue = (leftInt ^ rightInt);
                break;
            }
            case TokenOR: {
                int leftInt = value.v.floatValue;
                int rightInt = rightValue.v.floatValue;
                newValue.v.floatValue = (leftInt | rightInt);
                break;
            }
            case TokenAND: {
                int leftInt = value.v.floatValue;
                int rightInt = rightValue.v.floatValue;
                newValue.v.floatValue = (leftInt & rightInt);
                break;
            }
            case TokenEq: {
                newValue.v.floatValue = (value.v.floatValue == rightValue.v.floatValue) ? -1.0f : 0.0f;
                break;
            }
            case TokenUneq: {
                newValue.v.floatValue = (value.v.floatValue != rightValue.v.floatValue) ? -1.0f : 0.0f;
                break;
            }
            case TokenGr: {
                newValue.v.floatValue = (value.v.floatValue > rightValue.v.floatValue) ? -1.0f : 0.0f;
                break;
            }
            case TokenLe: {
                newValue.v.floatValue = (value.v.floatValue < rightValue.v.floatValue) ? -1.0f : 0.0f;
                break;
            }
            case TokenGrEq: {
                newValue.v.floatValue = (value.v.floatValue >= rightValue.v.floatValue) ? -1.0f : 0.0f;
                break;
            }
            case TokenLeEq: {
                newValue.v.floatValue = (value.v.floatValue <= rightValue.v.floatValue) ? -1.0f : 0.0f;
                break;
            }
            case TokenPlus: {
                newValue.v.floatValue = value.v.floatValue + rightValue.v.floatValue;
                break;
            }
            case TokenMinus: {
                newValue.v.floatValue = value.v.floatValue - rightValue.v.floatValue;
                break;
            }
            case TokenMOD: {
                newValue.v.floatValue = (int)value.v.floatValue % (int)rightValue.v.floatValue;
                break;
            }
            case TokenMul: {
                newValue.v.floatValue = value.v.floatValue * rightValue.v.floatValue;
                break;
            }
            case TokenDiv: {
                newValue.v.floatValue = value.v.floatValue / rightValue.v.floatValue;
                break;
            }
            case TokenPow: {
                newValue.v.floatValue = powf(value.v.floatValue, rightValue.v.floatValue);
                break;
            }
            default: {
                newValue.type = ValueError;
                newValue.v.errorCode = ErrorSyntax;
            }
        }
        
        value = newValue;
        if (value.type == ValueError) break;
    }
    return value;
}

struct TypedValue LRC_evaluatePrimaryExpression(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    struct TypedValue value;
    switch (interpreter->pc->type)
    {
        case TokenFloat: {
            value.type = ValueFloat;
            value.v.floatValue = interpreter->pc->floatValue;
            ++interpreter->pc;
            break;
        }
        case TokenString: {
            value.type = ValueString;
            value.v.stringValue = interpreter->pc->stringValue;
            ++interpreter->pc;
            break;
        }
        case TokenIdentifier:
        case TokenStringIdentifier: {
            enum ErrorCode errorCode = ErrorNone;
            enum ValueType valueType = ValueNull;
            union Value *varValue = LRC_readVariable(core, &valueType, &errorCode);
            if (varValue)
            {
                value.type = valueType;
                value.v = *varValue;
            }
            else
            {
                value.type = ValueError;
                value.v.errorCode = errorCode;
            }
            break;
        }
        case TokenBracketOpen: {
            ++interpreter->pc;
            value = LRC_evaluateExpression(core);
            if (interpreter->pc->type != TokenBracketClose)
            {
                value.type = ValueError;
                value.v.errorCode = ErrorExpectedRightParenthesis;
            }
            else
            {
                ++interpreter->pc;
            }
            break;
        }
        default: {
            value.type = ValueError;
            value.v.errorCode = ErrorSyntax;
        }
    }
    return value;
}

int LRC_isEndOfCommand(struct Interpreter *interpreter)
{
    enum TokenType type = interpreter->pc->type;
    return (type == TokenEol || type == TokenELSE);
}

enum ErrorCode LRC_endOfCommand(struct Interpreter *interpreter)
{
    enum TokenType type = interpreter->pc->type;
    if (type == TokenEol)
    {
        interpreter->isSingleLineIf = false;
        ++interpreter->pc;
        return ErrorNone;
    }
    return (type == TokenELSE) ? ErrorNone : ErrorUnexpectedToken;
}

enum TokenType LRC_getNextTokenType(struct Interpreter *interpreter)
{
    return (interpreter->pc + 1)->type;
}

enum ErrorCode LRC_evaluateCommand(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    switch (interpreter->pc->type)
    {
        case TokenUndefined:
            return ErrorEndOfProgram;
            
        case TokenLabel:
            ++interpreter->pc;
            if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
            ++interpreter->pc;
            break;
        
        case TokenEol:
            interpreter->isSingleLineIf = false;
            ++interpreter->pc;
            break;
            
        case TokenEND:
            if (LRC_getNextTokenType(interpreter) == TokenIF)
            {
                return cmd_ENDIF(core);
            }
            return cmd_END(core);
            
        case TokenLET:
        case TokenIdentifier:
        case TokenStringIdentifier:
            return cmd_LET(core);
        
        case TokenPRINT:
            return cmd_PRINT(core);
        
        case TokenIF:
            return cmd_IF(core);
        
        case TokenELSE:
            return cmd_ELSE(core);

        case TokenFOR:
            return cmd_FOR(core);

        case TokenNEXT:
            return cmd_NEXT(core);

        case TokenGOTO:
            return cmd_GOTO(core);

        case TokenDATA:
        case TokenDIM:
        case TokenGOSUB:
        case TokenINPUT:
        case TokenON:
        case TokenPEEK:
        case TokenPOKE:
        case TokenRANDOMIZE:
        case TokenREAD:
        case TokenREM:
        case TokenRESTORE:
        case TokenRETURN:
        default:
            printf("Command not implemented: %s\n", TokenStrings[interpreter->pc->type]);
            return ErrorUnexpectedToken;
    }
    return ErrorNone;
}

void LRC_pushLabelStackItem(struct Interpreter *interpreter, enum LabelType type, struct Token *token)
{
    assert(interpreter->numLabelStackItems < MAX_LABEL_STACK_ITEMS);
    struct LabelStackItem *item = &interpreter->labelStackItems[interpreter->numLabelStackItems];
    item->type = type;
    item->token = token;
    interpreter->numLabelStackItems++;
}

struct LabelStackItem *LRC_popLabelStackItem(struct Interpreter *interpreter)
{
    if (interpreter->numLabelStackItems > 0)
    {
        interpreter->numLabelStackItems--;
        return &interpreter->labelStackItems[interpreter->numLabelStackItems];
    }
    return NULL;
}

struct LabelStackItem *LRC_peekLabelStackItem(struct Interpreter *interpreter)
{
    if (interpreter->numLabelStackItems > 0)
    {
        return &interpreter->labelStackItems[interpreter->numLabelStackItems - 1];
    }
    return NULL;
}

struct JumpLabelItem *LRC_getJumpLabel(struct Interpreter *interpreter, int symbolIndex)
{
    struct JumpLabelItem *item;
    for (int i = 0; i < interpreter->numJumpLabelItems; i++)
    {
        item = &interpreter->jumpLabelItems[i];
        if (item->symbolIndex == symbolIndex)
        {
            return item;
        }
    }
    return NULL;
}

enum ErrorCode LRC_setJumpLabel(struct Interpreter *interpreter, int symbolIndex, struct Token *token)
{
    if (LRC_getJumpLabel(interpreter, symbolIndex) != NULL)
    {
        return ErrorLabelAlreadyDefined;
    }
    if (interpreter->numJumpLabelItems >= MAX_JUMP_LABEL_ITEMS)
    {
        return ErrorTooManyLabels;
    }
    struct JumpLabelItem *item = &interpreter->jumpLabelItems[interpreter->numJumpLabelItems];
    item->symbolIndex = symbolIndex;
    item->token = token;
    interpreter->numJumpLabelItems++;
    return ErrorNone;
}
