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

enum ErrorCode cmd_SOUND(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SOUND
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // wave value
    struct TypedValue waveValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
    if (waveValue.type == ValueTypeError) return waveValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // pulse width value
    struct TypedValue pwValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (pwValue.type == ValueTypeError) return pwValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // length value
    struct TypedValue lenValue = itp_evaluateOptionalNumericExpression(core, 0, 255);
    if (lenValue.type == ValueTypeError) return lenValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (waveValue.type != ValueTypeNull)
        {
            voice->attr.wave = waveValue.v.floatValue;
        }
        if (pwValue.type != ValueTypeNull)
        {
            voice->attr.pulseWidth = pwValue.v.floatValue;
        }
        if (lenValue.type != ValueTypeNull)
        {
            int len = lenValue.v.floatValue;
            voice->length = len;
            voice->attr.timeout = (len > 0) ? 1 : 0;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_SOUND_SOURCE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // SOUND SOURCE
    ++interpreter->pc;
    ++interpreter->pc;
    
    // address value
    struct TypedValue aValue = itp_evaluateNumericExpression(core, 0, 0xFFFF);
    if (aValue.type == ValueTypeError) return aValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        interpreter->audioLib.soundSourceAddress = aValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

//enum ErrorCode cmd_SOUND_COPY(struct Core *core)
//{
//    struct Interpreter *interpreter = core->interpreter;
//    
//    // SOUND COPY
//    ++interpreter->pc;
//    ++interpreter->pc;
//    
//    // sound value
//    struct TypedValue sValue = itp_evaluateNumericExpression(core, 0, 15);
//    if (sValue.type == ValueTypeError) return sValue.v.errorCode;
//    
//    // TO
//    if (interpreter->pc->type != TokenTO) return ErrorExpectedTo;
//    ++interpreter->pc;
//    
//    // voice value
//    struct TypedValue vValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
//    if (vValue.type == ValueTypeError) return vValue.v.errorCode;
//
//    if (interpreter->pass == PassRun)
//    {
//        audlib_copySound(&interpreter->audioLib, sValue.v.floatValue, vValue.v.floatValue);
//    }
//    
//    return itp_endOfCommand(interpreter);
//}

enum ErrorCode cmd_VOLUME(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // VOLUME
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // volume value
    struct TypedValue volValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (volValue.type == ValueTypeError) return volValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // mix value
    struct TypedValue mixValue = itp_evaluateOptionalNumericExpression(core, 0, 3);
    if (mixValue.type == ValueTypeError) return mixValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (volValue.type != ValueTypeNull)
        {
            voice->status.volume = volValue.v.floatValue;
        }
        if (mixValue.type != ValueTypeNull)
        {
            int mix = mixValue.v.floatValue;
            voice->status.mixL = mix & 0x01;
            voice->status.mixR = (mix >> 1) & 0x01;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_ENVELOPE(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // ENVELOPE
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // attack value
    struct TypedValue attValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (attValue.type == ValueTypeError) return attValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // decay value
    struct TypedValue decValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (decValue.type == ValueTypeError) return decValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // sustain value
    struct TypedValue susValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (susValue.type == ValueTypeError) return susValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // release value
    struct TypedValue relValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (relValue.type == ValueTypeError) return relValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (attValue.type != ValueTypeNull)
        {
            voice->envA = attValue.v.floatValue;
        }
        if (decValue.type != ValueTypeNull)
        {
            voice->envD = decValue.v.floatValue;
        }
        if (susValue.type != ValueTypeNull)
        {
            voice->envS = susValue.v.floatValue;
        }
        if (relValue.type != ValueTypeNull)
        {
            voice->envR = relValue.v.floatValue;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_LFO(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // LFO
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // rate value
    struct TypedValue rateValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (rateValue.type == ValueTypeError) return rateValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // osc amount value
    struct TypedValue oscValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (oscValue.type == ValueTypeError) return oscValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // vol amount value
    struct TypedValue volValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (volValue.type == ValueTypeError) return volValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // pw amount value
    struct TypedValue pwValue = itp_evaluateOptionalNumericExpression(core, 0, 15);
    if (pwValue.type == ValueTypeError) return pwValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        int n = nValue.v.floatValue;
        struct Voice *voice = &core->machine->audioRegisters.voices[n];
        if (rateValue.type != ValueTypeNull)
        {
            voice->lfoFrequency = rateValue.v.floatValue;
        }
        if (oscValue.type != ValueTypeNull)
        {
            voice->lfoOscAmount = oscValue.v.floatValue;
        }
        if (volValue.type != ValueTypeNull)
        {
            voice->lfoVolAmount = volValue.v.floatValue;
        }
        if (pwValue.type != ValueTypeNull)
        {
            voice->lfoPWAmount = pwValue.v.floatValue;
        }
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_LFO_A(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // LFO.A
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
    
    union LFOAttributes attr;
    if (voice) attr = voice->lfoAttr; else attr.value = 0;
    
    // attr value
    struct TypedValue attrValue = itp_evaluateLFOAttributes(core, attr);
    if (attrValue.type == ValueTypeError) return attrValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        voice->lfoAttr.value = attrValue.v.floatValue;
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_PLAY(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // PLAY
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    // comma
    if (interpreter->pc->type != TokenComma) return ErrorExpectedComma;
    ++interpreter->pc;
    
    // pitch value
    struct TypedValue pValue = itp_evaluateNumericExpression(core, 0, 96);
    if (pValue.type == ValueTypeError) return pValue.v.errorCode;
    
    int len = -1;
    if (interpreter->pc->type == TokenComma)
    {
        // comma
        ++interpreter->pc;
        
        // length value
        struct TypedValue lenValue = itp_evaluateNumericExpression(core, 0, 255);
        if (lenValue.type == ValueTypeError) return lenValue.v.errorCode;
        
        len = lenValue.v.floatValue;
    }
    
    int sound = -1;
    if (interpreter->pc->type == TokenSOUND)
    {
        // SOUND
        ++interpreter->pc;

        // length value
        struct TypedValue sValue = itp_evaluateNumericExpression(core, 0, 15);
        if (sValue.type == ValueTypeError) return sValue.v.errorCode;
        
        sound = sValue.v.floatValue;
    }
    
    if (interpreter->pass == PassRun)
    {
        audlib_play(&core->interpreter->audioLib, nValue.v.floatValue, pValue.v.floatValue, len, sound);
    }
    
    return itp_endOfCommand(interpreter);
}

enum ErrorCode cmd_STOP(struct Core *core)
{
    struct Interpreter *interpreter = core->interpreter;
    
    // STOP
    ++interpreter->pc;
    
    // n value
    struct TypedValue nValue = itp_evaluateOptionalNumericExpression(core, 0, NUM_VOICES - 1);
    if (nValue.type == ValueTypeError) return nValue.v.errorCode;
    
    if (interpreter->pass == PassRun)
    {
        if (nValue.type != ValueTypeNull)
        {
            int n = nValue.v.floatValue;
            struct Voice *voice = &core->machine->audioRegisters.voices[n];
            voice->status.gate = 0;
        }
        else
        {
            for (int i = 0; i < NUM_VOICES; i++)
            {
                struct Voice *voice = &core->machine->audioRegisters.voices[i];
                voice->status.gate = 0;
            }
        }
    }
    
    return itp_endOfCommand(interpreter);
}
