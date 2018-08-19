//
// Copyright 2018 Timo Kloss
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

#include "cmd_audio.h"
#include "core.h"
#include "interpreter_utils.h"
#include <assert.h>

enum ErrorCode cmd_VOICE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // VOICE
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // frequency value
    struct TypedValue fValue = itp_evaluateOptionalNumericExpression(core, 0, 65535);
    if (fValue.type == ValueTypeError) return fValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // volume value
    struct TypedValue volValue = itp_evaluateOptionalNumericExpression(core, 0, 255);
    if (volValue.type == ValueTypeError) return volValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // pulse width value
    struct TypedValue pwValue = itp_evaluateOptionalNumericExpression(core, 0, 255);
    if (pwValue.type == ValueTypeError) return pwValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (fValue.type != ValueTypeNull)
        {
            int f = fValue.v.floatValue;
            voice->frequencyLow = f & 0xFF;
            voice->frequencyHigh = f >> 8;
        }
        if (volValue.type != ValueTypeNull)
        {
            voice->volume = volValue.v.floatValue;
        }
        if (pwValue.type != ValueTypeNull)
        {
            voice->pulseWidth = pwValue.v.floatValue;
        }
        
        if (!core->machine->audioRegisters.attr.audioEnabled)
        {
            core->machine->audioRegisters.attr.audioEnabled = 1;
            delegate_controlsDidChange(core);
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_VOICE_A(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // VOICE.A
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    struct Voice *voice = NULL;
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        voice = &core->machine->audioRegisters.voices[n];
    }
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    union VoiceAttributes attr;
    if (voice)
    {
        attr = voice->attr;
    }
    else
    {
        attr.value = 0;
    }
    
    // attr value
    struct TypedValue aValue = itp_evaluateVoiceAttributes(core, attr);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        voice->attr.value = aValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

struct TypedValue fnc_VOICE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // VOICE.?
    enum TokenType type = interpreter->pc->type;
    ++interpreter->pc;
    
    // bracket open
    if (interpreter->pc->type != TokenBracketOpen) return val_makeError(ErrorExpectedLeftParenthesis);
        ++interpreter->pc;
    
    // expression
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue;
    
    // bracket close
    if (interpreter->pc->type != TokenBracketClose) return val_makeError(ErrorExpectedRightParenthesis);
        ++interpreter->pc;
    
    struct TypedValue value;
    value.type = ValueTypeFloat;
    
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        switch (type)
        {
            case TokenVOICEF:
                value.v.floatValue = (voice->frequencyHigh << 8) | voice->frequencyLow;
                break;

            case TokenVOICEA:
                value.v.floatValue = voice->attr.value;
                break;
                
            case TokenVOICEPW:
                value.v.floatValue = voice->pulseWidth;
                break;
                
            case TokenVOICEV:
                value.v.floatValue = voice->volume;
                break;
                
            default:
                assert(0);
                break;
        }
    }
    return value;
}
