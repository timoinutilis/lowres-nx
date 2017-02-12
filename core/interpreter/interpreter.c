//
// Copyright 2016 Timo Kloss
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
#include <string.h>
#include "lowres_core.h"

const char *TokenStrings[] = {
    NULL,
    
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    
    ":",
    ",",
    ";",
    NULL,
    
    "=",
    ">=",
    "<=",
    "<>",
    ">",
    "<",
    "(",
    ")",
    "+",
    "-",
    "*",
    "/",
    "^",
    
    "AND",
    "DATA",
    "DIM",
    "ELSE",
    "END",
    "FOR",
    "GOSUB",
    "GOTO",
    "IF",
    "INPUT",
    "LET",
    "MOD",
    "NEXT",
    "NOT",
    "ON",
    "OR",
    "PEEK",
    "POKE",
    "PRINT",
    "RANDOMIZE",
    "READ",
    "REM",
    "RESTORE",
    "RETURN",
    "STEP",
    "THEN",
    "TO",
    "XOR"
};

const char *ErrorStrings[] = {
    "OK",
    "Too Many Tokens",
    "Expected End Of String",
    "Unexpected Character",
    "Syntax Error",
    "End Of Program",
    "Unexpected Token",
    "Expected End Of Line",
    "Expected THEN",
    "Expected Equal Sign '='",
    "Expected Variable Identifier",
    "Expected Right Parenthesis ')'",
    "Symbol Name Too Long",
    "Too Many Symbols",
    "Type Mismatch",
    "Out Of Memory"
};

union Value *LRC_readVariable(struct LowResCore *core, enum ErrorCode *errorCode);
struct TypedValue LRC_evaluateExpression(struct LowResCore *core);
enum ErrorCode LRC_endOfCommand(struct Interpreter *interpreter);
enum ErrorCode LRC_runCommand(struct LowResCore *core);


enum ErrorCode LRC_tokenizeProgram(struct LowResCore *core, const char *sourceCode)
{
    const char *charSetDigits = "0123456789";
    const char *charSetLetters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_";
    const char *charSetAlphaNum = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789";
    
    struct Interpreter *interpreter = &core->interpreter;
    const char *character = sourceCode;
    
    uint8_t *currentRomByte = core->machine.cartridgeRom;
    
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
            memcpy(currentRomByte, firstCharacter, len);
            token->type = TokenString;
            token->stringValue = (const char *)currentRomByte;
            currentRomByte += len;
            *currentRomByte++ = 0;
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

enum ErrorCode LRC_runProgram(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    interpreter->pc = interpreter->tokens;
    interpreter->simpleVariablesEnd = (struct SimpleVariable *)interpreter->variablesStack;
    enum ErrorCode errorCode = ErrorNone;
    
    do
    {
        errorCode = LRC_runCommand(core);
    }
    while (errorCode == ErrorNone);
    
    return errorCode;
}

union Value *LRC_readVariable(struct LowResCore *core, enum ErrorCode *errorCode)
{
    struct Interpreter *interpreter = &core->interpreter;
    int symbolIndex = interpreter->pc->symbolIndex;
    ++interpreter->pc;
    
    struct SimpleVariable *variable = (struct SimpleVariable *)interpreter->variablesStack;
    while (variable < interpreter->simpleVariablesEnd)
    {
        if (variable->symbolIndex == symbolIndex)
        {
            // variable found
            return &variable->v;
        }
        variable++;
    }
    
    // create new variable
    variable = interpreter->simpleVariablesEnd;
    interpreter->simpleVariablesEnd++;
    if ((void *)(interpreter->simpleVariablesEnd) >= (void *)&interpreter->variablesStack[VARIABLES_STACK_SIZE])
    {
        *errorCode = ErrorOutOfMemory;
        return NULL;
    }
    memset(variable, 0, sizeof(struct SimpleVariable));
    variable->symbolIndex = symbolIndex;
    return &variable->v;
    
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
    struct Interpreter *interpreter = &core->interpreter;
    struct TypedValue value;
    switch (interpreter->pc->type)
    {
        case TokenFloat:
            value.type = ValueFloat;
            value.v.floatValue = interpreter->pc->floatValue;
            ++interpreter->pc;
            break;
            
        case TokenString:
            value.type = ValueString;
            value.v.stringValue = interpreter->pc->stringValue;
            ++interpreter->pc;
            break;
            
        case TokenIdentifier: {
            enum ErrorCode errorCode = ErrorNone;
            union Value *varValue = LRC_readVariable(core, &errorCode);
            if (varValue)
            {
                value.type = ValueFloat;
                value.v.floatValue = varValue->floatValue;
            }
            else
            {
                value.type = ValueError;
                value.v.errorCode = errorCode;
            }
            break;
        }
        case TokenStringIdentifier:
            value.type = ValueString;
            value.v.stringValue = "VARIABLE STRING";
            ++interpreter->pc;
            break;

        default:
            value.type = ValueError;
            value.v.errorCode = ErrorSyntax;
    }
    return value;
}

enum ErrorCode LRC_endOfCommand(struct Interpreter *interpreter)
{
    enum TokenType type = interpreter->pc->type;
    if (type == TokenEol)
    {
        ++interpreter->pc;
        return ErrorNone;
    }
    return (type == TokenELSE) ? ErrorNone : ErrorUnexpectedToken;
}

enum ErrorCode LRC_runCommand(struct LowResCore *core)
{
    struct Interpreter *interpreter = &core->interpreter;
    switch (interpreter->pc->type)
    {
        case TokenUndefined:
            return ErrorEndOfProgram;
            
        case TokenEND:
            ++interpreter->pc;
            return LRC_endOfCommand(interpreter) ?: ErrorEndOfProgram;
            
        case TokenLET:
            ++interpreter->pc;
            if (interpreter->pc->type != TokenIdentifier && interpreter->pc->type != TokenStringIdentifier) return ErrorExpectedVariableIdentifier;
            // fall through
        case TokenIdentifier:
        case TokenStringIdentifier: {
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
        case TokenLabel: {
            ++interpreter->pc;
            if (interpreter->pc->type != TokenEol) return ErrorExpectedEndOfLine;
            ++interpreter->pc;
            break;
        }
        case TokenEol: {
            ++interpreter->pc;
            break;
        }
        case TokenPRINT: {
            ++interpreter->pc;
            struct TypedValue value = LRC_evaluateExpression(core);
            if (value.type == ValueError) return value.v.errorCode;
            if (value.type == ValueString)
            {
                printf("print string: %s\n", value.v.stringValue);
            }
            else if (value.type == ValueFloat)
            {
                printf("print float: %f\n", value.v.floatValue);
            }
            else
            {
                printf("print unknown\n");
            }
            return LRC_endOfCommand(interpreter);
        }
        case TokenIF: {
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
            break;
        }
        case TokenELSE: {
            while (interpreter->pc->type != TokenEol)
            {
                ++interpreter->pc;
            }
            ++interpreter->pc;
            break;
        }
        case TokenDATA:
        case TokenDIM:
        case TokenFOR:
        case TokenGOSUB:
        case TokenGOTO:
        case TokenINPUT:
        case TokenNEXT:
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
