//
//  interpreter_utils.c
//  LowRes NX macOS
//
//  Created by Timo Kloss on 11/5/17.
//  Copyright © 2017 Inutilis Software. All rights reserved.
//

#include "interpreter_utils.h"
#include "core.h"

struct TypedValue itp_evaluateCharAttributes(struct Core *core, union CharacterAttributes oldAttr, bool isOptional)
{
    struct Interpreter *interpreter = &core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;
        
        union CharacterAttributes resultAttr = oldAttr;
        
        // palette value
        struct TypedValue palValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
        if (palValue.type == ValueTypeError) return palValue;
        
        // comma
        if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
        ++interpreter->pc;
        
        // bank value
        struct TypedValue bValue = itp_evaluateOptionalNumericExpression(core, 0, 1);
        if (bValue.type == ValueTypeError) return bValue;
        
        // comma
        if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
        ++interpreter->pc;
        
        // flip x value
        struct TypedValue fxValue = itp_evaluateOptionalNumericExpression(core, 0, 1);
        if (fxValue.type == ValueTypeError) return fxValue;
        
        // comma
        if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
        ++interpreter->pc;
        
        // flip y value
        struct TypedValue fyValue = itp_evaluateOptionalNumericExpression(core, 0, 1);
        if (fyValue.type == ValueTypeError) return fyValue;
        
        // comma
        if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
        ++interpreter->pc;
        
        // priority value
        struct TypedValue priValue = itp_evaluateOptionalNumericExpression(core, 0, 1);
        if (priValue.type == ValueTypeError) return priValue;
        
        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
        interpreter->pc++;
        
        if (interpreter->pass == PassRun)
        {
            if (palValue.type != ValueTypeNull) resultAttr.palette = palValue.v.floatValue;
            if (bValue.type != ValueTypeNull) resultAttr.bank = bValue.v.floatValue;
            if (fxValue.type != ValueTypeNull) resultAttr.flipX = fxValue.v.floatValue;
            if (fyValue.type != ValueTypeNull) resultAttr.flipY = fyValue.v.floatValue;
            if (priValue.type != ValueTypeNull) resultAttr.priority = priValue.v.floatValue;
        }
        
        struct TypedValue resultValue;
        resultValue.type = ValueTypeFloat;
        resultValue.v.floatValue = resultAttr.value;
        return resultValue;
    }
    else if (isOptional)
    {
        return itp_evaluateOptionalNumericExpression(core, 0, 255);
    }
    else
    {
        return itp_evaluateNumericExpression(core, 0, 255);
    }
}

struct TypedValue itp_evaluateSpriteSizeAttributes(struct Core *core, union SpriteSizeAttributes oldAttr, bool isOptional)
{
    struct Interpreter *interpreter = &core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;
        
        union SpriteSizeAttributes resultAttr = oldAttr;
        
        // width value
        struct TypedValue wValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
        if (wValue.type == ValueTypeError) return wValue;
        
        // comma
        if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
        ++interpreter->pc;
        
        // height value
        struct TypedValue hValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
        if (hValue.type == ValueTypeError) return hValue;
        
        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
        interpreter->pc++;
        
        if (interpreter->pass == PassRun)
        {
            if (wValue.type != ValueTypeNull) resultAttr.width = wValue.v.floatValue;
            if (hValue.type != ValueTypeNull) resultAttr.height = hValue.v.floatValue;
        }
        
        struct TypedValue resultValue;
        resultValue.type = ValueTypeFloat;
        resultValue.v.floatValue = resultAttr.value;
        return resultValue;
    }
    else if (isOptional)
    {
        return itp_evaluateOptionalNumericExpression(core, 0, 255);
    }
    else
    {
        return itp_evaluateNumericExpression(core, 0, 255);
    }
}

struct TypedValue itp_evaluateDisplayAttributes(struct Core *core, union DisplayAttributes oldAttr)
{
    struct Interpreter *interpreter = &core->interpreter;
    if (interpreter->pc->type == TokenBracketOpen)
    {
        // bracket open
        interpreter->pc++;
        
        union DisplayAttributes resultAttr = oldAttr;
        
        // sprites value
        struct TypedValue sValue = itp_evaluateOptionalNumericExpression(core, 0, 1);
        if (sValue.type == ValueTypeError) return sValue;

        // comma
        if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
        ++interpreter->pc;
        
        // bg0 value
        struct TypedValue bg0Value = itp_evaluateOptionalNumericExpression(core, 0, 1);
        if (bg0Value.type == ValueTypeError) return bg0Value;

        // comma
        if (interpreter->pc->type != TokenComma) return val_makeError(ErrorExpectedComma);
        ++interpreter->pc;
        
        // bg1 value
        struct TypedValue bg1Value = itp_evaluateOptionalNumericExpression(core, 0, 1);
        if (bg1Value.type == ValueTypeError) return bg1Value;
        
        // bracket close
        if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
        interpreter->pc++;
        
        if (interpreter->pass == PassRun)
        {
            if (sValue.type != ValueTypeNull) resultAttr.spritesEnabled = sValue.v.floatValue;
            if (bg0Value.type != ValueTypeNull) resultAttr.planeAEnabled = bg0Value.v.floatValue;
            if (bg1Value.type != ValueTypeNull) resultAttr.planeBEnabled = bg1Value.v.floatValue;
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
