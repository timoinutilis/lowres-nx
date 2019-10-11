//
// Copyright 2017-2018 Timo Kloss
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

#include "interpreter_utils.h"
#include "core.h"

enum ErrorCode itp_evaluateSimpleAttributes(struct Core *core, struct SimpleAttributes *attrs)
{
    struct Interpreter *interpreter = core->interpreter;
    
    attrs->pal = -1;
    attrs->flipX = -1;
    attrs->flipY = -1;
    attrs->prio = -1;
    attrs->size = -1;
    
    bool changed = false;
    bool checked = false;
    
    do
    {
        checked = false;
        
        // PAL
        if (interpreter->pc->type == TokenPAL && attrs->pal == -1)
        {
            ++interpreter->pc;
            
            struct TypedValue value = itp_evaluateNumericExpression(core, 0, NUM_PALETTES - 1);
            if (value.type == ValueTypeError) return value.v.errorCode;
            attrs->pal = value.v.floatValue;
            
            checked = true;
        }
        
        // FLIP
        if (interpreter->pc->type == TokenFLIP && attrs->flipX == -1)
        {
            ++interpreter->pc;
            
            struct TypedValue fxValue = itp_evaluateNumericExpression(core, -1, 1);
            if (fxValue.type == ValueTypeError) return fxValue.v.errorCode;
            attrs->flipX = fxValue.v.floatValue ? 1 : 0;
            
            // comma
            if (interpreter->pc->type != TokenComma) return ErrorSyntax;
            ++interpreter->pc;
            
            struct TypedValue fyValue = itp_evaluateNumericExpression(core, -1, 1);
            if (fyValue.type == ValueTypeError) return fyValue.v.errorCode;
            attrs->flipY = fyValue.v.floatValue ? 1 : 0;
            
            checked = true;
        }
        
        // PRIO
        if (interpreter->pc->type == TokenPRIO && attrs->prio == -1)
        {
            ++interpreter->pc;
            
            struct TypedValue value = itp_evaluateNumericExpression(core, -1, 1);
            if (value.type == ValueTypeError) return value.v.errorCode;
            attrs->prio = value.v.floatValue ? 1 : 0;
            
            checked = true;
        }
        
        // SIZE
        if (interpreter->pc->type == TokenSIZE && attrs->size == -1)
        {
            ++interpreter->pc;
            
            struct TypedValue value = itp_evaluateNumericExpression(core, 0, 3);
            if (value.type == ValueTypeError) return value.v.errorCode;
            attrs->size = value.v.floatValue;
            
            checked = true;
        }
        
        changed |= checked;
    }
    while (checked);
    
    if (!changed) return ErrorSyntax;
    
    return ErrorNone;
}

struct TypedValue itp_evaluateCharAttributes(struct Core *core, union CharacterAttributes oldAttr)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;
        
        // obsolete syntax!
        
        union CharacterAttributes resultAttr = oldAttr;
        
        struct TypedValue palValue = {ValueTypeNull, 0};
        struct TypedValue fxValue = {ValueTypeNull, 0};
        struct TypedValue fyValue = {ValueTypeNull, 0};
        struct TypedValue priValue = {ValueTypeNull, 0};
        struct TypedValue sValue = {ValueTypeNull, 0};
        
        // palette value
        palValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_PALETTES - 1);
        if (palValue.type == ValueTypeError) return palValue;
        
        // comma
        if (interpreter->pc->type == TokenComma)
        {
            ++interpreter->pc;
            
            // flip x value
            fxValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
            if (fxValue.type == ValueTypeError) return fxValue;
            
            // comma
            if (interpreter->pc->type == TokenComma)
            {
                ++interpreter->pc;
                
                // flip y value
                fyValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                if (fyValue.type == ValueTypeError) return fyValue;
                
                // comma
                if (interpreter->pc->type == TokenComma)
                {
                    ++interpreter->pc;
                    
                    // priority value
                    priValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                    if (priValue.type == ValueTypeError) return priValue;

                    // comma
                    if (interpreter->pc->type == TokenComma)
                    {
                        ++interpreter->pc;
                        
                        // size value
                        sValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
                        if (sValue.type == ValueTypeError) return sValue;
                    }
                }
            }
        }
        
        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        interpreter->pc++;
        
        if (interpreter->pass == PassRun)
        {
            if (palValue.type != ValueTypeNull) resultAttr.palette = palValue.v.floatValue;
            if (fxValue.type != ValueTypeNull) resultAttr.flipX = fxValue.v.floatValue;
            if (fyValue.type != ValueTypeNull) resultAttr.flipY = fyValue.v.floatValue;
            if (priValue.type != ValueTypeNull) resultAttr.priority = priValue.v.floatValue;
            if (sValue.type != ValueTypeNull) resultAttr.size = sValue.v.floatValue;
        }
        
        struct TypedValue resultValue;
        resultValue.type = ValueTypeFloat;
        resultValue.v.floatValue = resultAttr.value;
        return resultValue;
    }
    else
    {
        return itp_evaluateNumericExpression(core, 0, 255);
    }
}

struct TypedValue itp_evaluateDisplayAttributes(struct Core *core, union DisplayAttributes oldAttr)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;
        
        union DisplayAttributes resultAttr = oldAttr;
        
        struct TypedValue sValue = {ValueTypeNull, 0};
        struct TypedValue bg0Value = {ValueTypeNull, 0};
        struct TypedValue bg1Value = {ValueTypeNull, 0};
        struct TypedValue bg0SizeValue = {ValueTypeNull, 0};
        struct TypedValue bg1SizeValue = {ValueTypeNull, 0};

        // sprites value
        sValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
        if (sValue.type == ValueTypeError) return sValue;

        // comma
        if (interpreter->pc->type == TokenComma)
        {
            ++interpreter->pc;
            
            // bg0 value
            bg0Value = itp_evaluateOptionalNumericExpression(core, -1, 1);
            if (bg0Value.type == ValueTypeError) return bg0Value;

            // comma
            if (interpreter->pc->type == TokenComma)
            {
                ++interpreter->pc;
                
                // bg1 value
                bg1Value = itp_evaluateOptionalNumericExpression(core, -1, 1);
                if (bg1Value.type == ValueTypeError) return bg1Value;
                
                // comma
                if (interpreter->pc->type == TokenComma)
                {
                    ++interpreter->pc;
                    
                    // bg0 cell size value
                    bg0SizeValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                    if (bg0SizeValue.type == ValueTypeError) return bg0SizeValue;
                    
                    // comma
                    if (interpreter->pc->type == TokenComma)
                    {
                        ++interpreter->pc;
                        
                        // bg1 cell size value
                        bg1SizeValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                        if (bg1SizeValue.type == ValueTypeError) return bg1SizeValue;
                    }
                }
            }
        }
        
        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        interpreter->pc++;
        
        if (interpreter->pass == PassRun)
        {
            if (sValue.type != ValueTypeNull) resultAttr.spritesEnabled = sValue.v.floatValue;
            if (bg0Value.type != ValueTypeNull) resultAttr.planeAEnabled = bg0Value.v.floatValue;
            if (bg1Value.type != ValueTypeNull) resultAttr.planeBEnabled = bg1Value.v.floatValue;
            if (bg0SizeValue.type != ValueTypeNull) resultAttr.planeACellSize = bg0SizeValue.v.floatValue;
            if (bg1SizeValue.type != ValueTypeNull) resultAttr.planeBCellSize = bg1SizeValue.v.floatValue;
        }
        
        struct TypedValue resultValue;
        resultValue.type = ValueTypeFloat;
        resultValue.v.floatValue = resultAttr.value;
        return resultValue;
    }
    else
    {
        return itp_evaluateNumericExpression(core, 0, 255);
    }
}

struct TypedValue itp_evaluateLFOAttributes(struct Core *core, union LFOAttributes oldAttr)
{
    struct Interpreter *interpreter = core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;
        
        union LFOAttributes resultAttr = oldAttr;
        
        struct TypedValue wavValue = {ValueTypeNull, 0};
        struct TypedValue invValue = {ValueTypeNull, 0};
        struct TypedValue envValue = {ValueTypeNull, 0};
        struct TypedValue triValue = {ValueTypeNull, 0};
        
        // wave value
        wavValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
        if (wavValue.type == ValueTypeError) return wavValue;
        
        // comma
        if (interpreter->pc->type == TokenComma)
        {
            ++interpreter->pc;
            
            // invert value
            invValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
            if (invValue.type == ValueTypeError) return invValue;
            
            // comma
            if (interpreter->pc->type == TokenComma)
            {
                ++interpreter->pc;
                
                // env mode value
                envValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                if (envValue.type == ValueTypeError) return envValue;
                
                // comma
                if (interpreter->pc->type == TokenComma)
                {
                    ++interpreter->pc;
                    
                    // trigger value
                    triValue = itp_evaluateOptionalNumericExpression(core, -1, 1);
                    if (triValue.type == ValueTypeError) return triValue;
                }
            }
        }
        
        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorSyntax);
        interpreter->pc++;
        
        if (interpreter->pass == PassRun)
        {
            if (wavValue.type != ValueTypeNull) resultAttr.wave = wavValue.v.floatValue;
            if (invValue.type != ValueTypeNull) resultAttr.invert = invValue.v.floatValue;
            if (envValue.type != ValueTypeNull) resultAttr.envMode = envValue.v.floatValue;
            if (triValue.type != ValueTypeNull) resultAttr.trigger = triValue.v.floatValue;
        }
        
        struct TypedValue resultValue;
        resultValue.type = ValueTypeFloat;
        resultValue.v.floatValue = resultAttr.value;
        return resultValue;
    }
    else
    {
        return itp_evaluateNumericExpression(core, 0, 255);
    }
}

