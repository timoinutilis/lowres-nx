//
// Copyright 2016-2017 Timo Kloss
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

#include "tokenizer.h"
#include "error.h"
#include "charsets.h"
#include <string.h>
#include <stdlib.h>
#include "string_utils.h"

struct CoreError tok_tokenizeProgram(struct Tokenizer *tokenizer, const char *sourceCode)
{
    const char *uppercaseSourceCode = uppercaseString(sourceCode);
    if (!uppercaseSourceCode) return err_makeCoreError(ErrorOutOfMemory, -1);
    
    struct CoreError error = tok_tokenizeUppercaseProgram(tokenizer, uppercaseSourceCode);
    free((void *)uppercaseSourceCode);
    
    return error;
}

struct CoreError tok_tokenizeUppercaseProgram(struct Tokenizer *tokenizer, const char *sourceCode)
{
    const char *character = sourceCode;
    
    // PROGRAM
    
    while (*character && *character != '#')
    {
        int tokenSourcePosition = (int)(character - sourceCode);
        if (tokenizer->numTokens >= MAX_TOKENS - 1)
        {
            return err_makeCoreError(ErrorTooManyTokens, tokenSourcePosition);
        }
        struct Token *token = &tokenizer->tokens[tokenizer->numTokens];
        token->sourcePosition = tokenSourcePosition;
        
        // line break \n or \n\r
        if (*character == '\n')
        {
            token->type = TokenEol;
            tokenizer->numTokens++;
            character++;
            if (*character == '\r') { character++; }
            continue;
        }
        
        // line break \r or \r\n
        if (*character == '\r')
        {
            token->type = TokenEol;
            tokenizer->numTokens++;
            character++;
            if (*character == '\n') { character++; }
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
                if (*character == '\n')
                {
                    return err_makeCoreError(ErrorUnterminatedString, (int)(character - sourceCode));
                }
                else if (*character < 0)
                {
                    return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - sourceCode));
                }
                character++;
            }
            int len = (int)(character - firstCharacter);
            struct RCString *string = rcstring_new(firstCharacter, len);
            if (!string) return err_makeCoreError(ErrorOutOfMemory, tokenSourcePosition);
            token->type = TokenString;
            token->stringValue = string;
            tokenizer->numTokens++;
            character++;
            continue;
        }
        
        // number
        if (strchr(CharSetDigits, *character))
        {
            float number = 0;
            int afterDot = 0;
            while (*character)
            {
                if (strchr(CharSetDigits, *character))
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
            tokenizer->numTokens++;
            continue;
        }
        
        // hex number
        if (*character == '$')
        {
            character++;
            int number = 0;
            while (*character)
            {
                char *spos = strchr(CharSetHex, *character);
                if (spos)
                {
                    int digit = (int)(spos - CharSetHex);
                    number <<= 4;
                    number += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            tokenizer->numTokens++;
            continue;
        }
        
        // bin number
        if (*character == '%')
        {
            character++;
            int number = 0;
            while (*character)
            {
                if (*character == '0' || *character == '1')
                {
                    int digit = *character - '0';
                    number <<= 1;
                    number += digit;
                    character++;
                }
                else
                {
                    break;
                }
            }
            token->type = TokenFloat;
            token->floatValue = number;
            tokenizer->numTokens++;
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
                int keywordIsAlphaNum = strchr(CharSetAlphaNum, keyword[0]) != NULL;
                for (int pos = 0; pos <= keywordLen; pos++)
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
                    else if (keywordIsAlphaNum && textCharacter && strchr(CharSetAlphaNum, textCharacter))
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
            if (foundKeywordToken == TokenREM || foundKeywordToken == TokenApostrophe)
            {
                // REM comment, skip until end of line
                while (*character)
                {
                    if (*character < 0)
                    {
                        return err_makeCoreError(ErrorUnexpectedCharacter, (int)(character - sourceCode));
                    }
                    if (*character == '\n')
                    {
                        character++;
                        break;
                    }
                    character++;
                }
            }
            else if (foundKeywordToken > Token_reserved)
            {
                return err_makeCoreError(ErrorReservedKeyword, tokenSourcePosition);
            }
            token->type = foundKeywordToken;
            tokenizer->numTokens++;
            continue;
        }
        
        // Symbol
        if (strchr(CharSetLetters, *character))
        {
            const char *firstCharacter = character;
            char isString = 0;
            while (*character)
            {
                if (strchr(CharSetAlphaNum, *character))
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
            if (tokenizer->numSymbols >= MAX_SYMBOLS)
            {
                return err_makeCoreError(ErrorTooManySymbols, tokenSourcePosition);
            }
            int len = (int)(character - firstCharacter);
            if (len >= SYMBOL_NAME_SIZE)
            {
                return err_makeCoreError(ErrorSymbolNameTooLong, tokenSourcePosition);
            }
            char symbolName[SYMBOL_NAME_SIZE];
            memcpy(symbolName, firstCharacter, len);
            symbolName[len] = 0;
            int symbolIndex = -1;
            // find existing symbol
            for (int i = 0; i < MAX_SYMBOLS && tokenizer->symbols[i].name[0] != 0; i++)
            {
                if (strcmp(symbolName, tokenizer->symbols[i].name) == 0)
                {
                    symbolIndex = i;
                    break;
                }
            }
            if (symbolIndex == -1)
            {
                // add new symbol
                strcpy(tokenizer->symbols[tokenizer->numSymbols].name, symbolName);
                symbolIndex = tokenizer->numSymbols++;
            }
            if (isString)
            {
                token->type = TokenStringIdentifier;
            }
            else if (*character == ':')
            {
                token->type = TokenLabel;
                character++;
                enum ErrorCode errorCode = tok_setJumpLabel(tokenizer, symbolIndex, token + 1);
                if (errorCode != ErrorNone) return err_makeCoreError(errorCode, tokenSourcePosition);
            }
            else
            {
                token->type = TokenIdentifier;
                if (tokenizer->numTokens > 0 && tokenizer->tokens[tokenizer->numTokens - 1].type == TokenSUB)
                {
                    enum ErrorCode errorCode = tok_setSub(tokenizer, symbolIndex, token + 1);
                    if (errorCode != ErrorNone) return err_makeCoreError(errorCode, tokenSourcePosition);
                }
            }
            token->symbolIndex = symbolIndex;
            tokenizer->numTokens++;
            continue;
        }
        
        // Unexpected character
        return err_makeCoreError(ErrorUnexpectedCharacter, tokenSourcePosition);
    }
    
    // add EOL to the end
    struct Token *token = &tokenizer->tokens[tokenizer->numTokens];
    token->sourcePosition = (int)(character - sourceCode);
    token->type = TokenEol;
    tokenizer->numTokens++;
    
    return err_noCoreError();
}

void tok_freeTokens(struct Tokenizer *tokenizer)
{
    // Free string tokens
    for (int i = 0; i < tokenizer->numTokens; i++)
    {
        struct Token *token = &tokenizer->tokens[i];
        if (token->type == TokenString)
        {
            rcstring_release(token->stringValue);
        }
    }
    memset(tokenizer, 0, sizeof(struct Tokenizer));
}

struct JumpLabelItem *tok_getJumpLabel(struct Tokenizer *tokenizer, int symbolIndex)
{
    struct JumpLabelItem *item;
    for (int i = 0; i < tokenizer->numJumpLabelItems; i++)
    {
        item = &tokenizer->jumpLabelItems[i];
        if (item->symbolIndex == symbolIndex)
        {
            return item;
        }
    }
    return NULL;
}

enum ErrorCode tok_setJumpLabel(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token)
{
    if (tok_getJumpLabel(tokenizer, symbolIndex) != NULL)
    {
        return ErrorLabelAlreadyDefined;
    }
    if (tokenizer->numJumpLabelItems >= MAX_JUMP_LABEL_ITEMS)
    {
        return ErrorTooManyLabels;
    }
    struct JumpLabelItem *item = &tokenizer->jumpLabelItems[tokenizer->numJumpLabelItems];
    item->symbolIndex = symbolIndex;
    item->token = token;
    tokenizer->numJumpLabelItems++;
    return ErrorNone;
}

struct SubItem *tok_getSub(struct Tokenizer *tokenizer, int symbolIndex)
{
    struct SubItem *item;
    for (int i = 0; i < tokenizer->numSubItems; i++)
    {
        item = &tokenizer->subItems[i];
        if (item->symbolIndex == symbolIndex)
        {
            return item;
        }
    }
    return NULL;
}

enum ErrorCode tok_setSub(struct Tokenizer *tokenizer, int symbolIndex, struct Token *token)
{
    if (tok_getSub(tokenizer, symbolIndex) != NULL)
    {
        return ErrorSubAlreadyDefined;
    }
    if (tokenizer->numSubItems >= MAX_SUB_ITEMS)
    {
        return ErrorTooManySubprograms;
    }
    struct SubItem *item = &tokenizer->subItems[tokenizer->numSubItems];
    item->symbolIndex = symbolIndex;
    item->token = token;
    tokenizer->numSubItems++;
    return ErrorNone;
}
