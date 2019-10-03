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

#include "cmd_sprites.h"
#include "core.h"
#include "value.h"
#include "cmd_text.h"
#include "interpreter_utils.h"
#include "sprites_lib.h"
#include <assert.h>

enum ErrorCode cmd_SPRITE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SPRITE
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;
        
        // x value
        struct TypedValue xValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
        if (xValue.type == ValueTypeError) return xValue.v.errorCode;
        
        // comma
        if (interpreter->pc->type != TokenComma) return ErrorSyntax;
        ++interpreter->pc;
        
        // y value
        struct TypedValue yValue = itp_evaluateOptionalExpression(core, TypeClassNumeric);
        if (yValue.type == ValueTypeError) return yValue.v.errorCode;

        // comma
        if (interpreter->pc->type != TokenComma) return ErrorSyntax;
        ++interpreter->pc;
        
        // c value
        struct TypedValue cValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_CHARACTERS - 1);
        if (cValue.type == ValueTypeError) return cValue.v.errorCode;

        if (interpreter->pass == PassRun)
        {
            int n = nValue.v.floatValue;
            struct Sprite *sprite = &core->machine->spriteRegisters.sprites[n];
            if (xValue.type != ValueTypeNull) sprite->x = ((int)xValue.v.floatValue + SPRITE_OFFSET_X) & 0xFF;
            if (yValue.type != ValueTypeNull) sprite->y = ((int)yValue.v.floatValue + SPRITE_OFFSET_Y) & 0xFF;
            if (cValue.type != ValueTypeNull) sprite->character = cValue.v.floatValue;
        }
    }
    else
    {
        struct SimpleAttributes attrs;
        enum ErrorCode attrsError = itp_evaluateSimpleAttributes(core, &attrs);
        if (attrsError != ErrorNone) return attrsError;
        
        if (interpreter->pass == PassRun)
        {
            int n = nValue.v.floatValue;
            struct Sprite *sprite = &core->machine->spriteRegisters.sprites[n];
            
            if (attrs.pal >= 0) sprite->attr.palette = attrs.pal;
            if (attrs.flipX >= 0) sprite->attr.flipX = attrs.flipX;
            if (attrs.flipY >= 0) sprite->attr.flipY = attrs.flipY;
            if (attrs.prio >= 0) sprite->attr.priority = attrs.prio;
            if (attrs.size >= 0) sprite->attr.size = attrs.size;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SPRITE_A(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SPRITE.A
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    struct Sprite *sprite = NULL;
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        sprite = &core->machine->spriteRegisters.sprites[n];
    }
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorSyntax;
    ++interpreter->pc;
    
    union CharacterAttributes attr;
    if (sprite)
    {
        attr = sprite->attr;
    }
    else
    {
        attr.value = 0;
    }
    
    // attr value
    struct TypedValue aValue = itp_evaluateCharAttributes(core, attr);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;

    if (interpreter->pass == PassRun)
    {
        sprite->attr.value = aValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SPRITE_OFF(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SPRITE
    ++interpreter->pc;
    
    // OFF
    if (interpreter->pc->type != TokenOFF) return ErrorSyntax;
    ++interpreter->pc;
    
    int from = 0;
    int to = NUM_SPRITES - 1;
    
    if (!itp_isEndOfCommand(interpreter))
    {
        // from value
        struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
        if (nValue.type == ValueTypeError) return nValue.v.errorCode;
        from = nValue.v.floatValue;
        to = from;
        
        // TO
        if (interpreter->pc->type == TokenTO)
        {
            ++interpreter->pc;
        
            // to value
            struct TypedValue mValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
            if (mValue.type == ValueTypeError) return mValue.v.errorCode;
            to = mValue.v.floatValue;
        }
    }
    
    if (interpreter->pass == PassRun)
    {
        for (int i = from; i <= to; i++)
        {
            struct Sprite *sprite = &core->machine->spriteRegisters.sprites[i];
            sprite->x = 0;
            sprite->y = 0;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_SPRITE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SPRITE.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // expression
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Sprite *sprite = &core->machine->spriteRegisters.sprites[n];
        switch (type)
        {
            case TokenSPRITEX:
                value.v.floatValue = sprite->x - SPRITE_OFFSET_X;
                break;
                
            case TokenSPRITEY:
                value.v.floatValue = sprite->y - SPRITE_OFFSET_Y;
                break;
                
            case TokenSPRITEC:
                value.v.floatValue = sprite->character;
                break;
                
            case TokenSPRITEA:
                value.v.floatValue = sprite->attr.value;
                break;

            default:
                assert(0);
                break;
        }
    }
    return value;
}

struct TypedValue fnc_SPRITE_HIT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SPRITE
    ++interpreter->pc;
    
    // HIT
    if (interpreter->pc->type != TokenHIT) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    // sprite number
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
    if (nValue.type == ValueTypeError) return nValue;
    
    int first = 0;
    int last = NUM_SPRITES - 1;
    
    // other sprite number
    if (interpreter->pc->type == TokenComma)
    {
        ++interpreter->pc;
        struct TypedValue otherValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
        if (otherValue.type == ValueTypeError) return otherValue;
        first = otherValue.v.floatValue;
        last = first;
        
        // last sprite number
        if (interpreter->pc->type == TokenTO)
        {
            ++interpreter->pc;
            struct TypedValue lastValue = itp_evaluateNumericExpression(core, 0, NUM_SPRITES - 1);
            if (lastValue.type == ValueTypeError) return lastValue;
            last = lastValue.v.floatValue;
        }
    }
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        bool hits = sprlib_checkCollision(&interpreter->spritesLib, nValue.v.floatValue, first, last);
        value.v.floatValue = hits ? BAS_TRUE : BAS_FALSE;
    }
    
    return value;
}

struct TypedValue fnc_HIT(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // HIT
    ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        value.v.floatValue = interpreter->spritesLib.lastHit;
    }
    
    return value;
}
