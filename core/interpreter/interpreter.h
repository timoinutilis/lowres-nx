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

#ifndef interpreter_h
#define interpreter_h

#include <stdio.h>
#include <stdbool.h>
#include "interpreter_config.h"
#include "tokenizer.h"
#include "token.h"
#include "error.h"
#include "value.h"
#include "labels.h"
#include "variables.h"
#include "data.h"
#include "text_lib.h"
#include "sprites_lib.h"
#include "audio_lib.h"
#include "io_chip.h"
#include "data_manager.h"

#define BAS_TRUE -1.0f
#define BAS_FALSE 0.0f

struct Core;

enum Pass {
    PassPrepare,
    PassRun
};

enum State {
    StateNoProgram,
    StateEvaluate,
    StateInput,
    StatePaused,
    StateWaitForDisk,
    StateEnd
};

enum Mode {
    ModeNone,
    ModeMain,
    ModeInterrupt
};

enum InterruptType {
    InterruptTypeRaster,
    InterruptTypeVBL
};

struct Interpreter {
    const char *sourceCode;
    
    enum Pass pass;
    enum State state;
    enum Mode mode;
    struct Token *pc;
    int subLevel;
    int cycles;
    int interruptOverCycles;
    bool debug;
    bool handlesPause;
    int cpuLoadDisplay;
    int cpuLoadMax;
    int cpuLoadTimer;
    
    struct Tokenizer tokenizer;
    
    struct DataManager romDataManager;
    
    struct LabelStackItem labelStackItems[MAX_LABEL_STACK_ITEMS];
    int numLabelStackItems;
    
    bool isSingleLineIf;
    
    struct SimpleVariable simpleVariables[MAX_SIMPLE_VARIABLES];
    int numSimpleVariables;
    struct ArrayVariable arrayVariables[MAX_ARRAY_VARIABLES];
    int numArrayVariables;
    struct RCString *nullString;
    
    struct Token *firstData;
    struct Token *lastData;
    struct Token *currentDataToken;
    struct Token *currentDataValueToken;
    
    struct Token *currentOnRasterToken;
    struct Token *currentOnVBLToken;
    
    int waitCount;
    bool exitEvaluation;
    union Gamepad lastFrameGamepads[NUM_GAMEPADS];
    union IOStatus lastFrameIOStatus;
    float timer;
    int seed;
    bool isKeyboardOptional;
    union Value *lastVariableValue;
    
    struct TextLib textLib;
    struct SpritesLib spritesLib;
    struct AudioLib audioLib;
};

void itp_init(struct Core *core);
void itp_deinit(struct Core *core);
struct CoreError itp_compileProgram(struct Core *core, const char *sourceCode);
void itp_runProgram(struct Core *core);
void itp_runInterrupt(struct Core *core, enum InterruptType type);
void itp_didFinishVBL(struct Core *core);
void itp_endProgram(struct Core *core);
void itp_freeProgram(struct Core *core);

enum ValueType itp_getIdentifierTokenValueType(struct Token *token);
union Value *itp_readVariable(struct Core *core, enum ValueType *type, enum ErrorCode *errorCode, bool forWriting);
struct TypedValue itp_evaluateExpression(struct Core *core, enum TypeClass typeClass);
struct TypedValue itp_evaluateNumericExpression(struct Core *core, int min, int max);
struct TypedValue itp_evaluateOptionalExpression(struct Core *core, enum TypeClass typeClass);
struct TypedValue itp_evaluateOptionalNumericExpression(struct Core *core, int min, int max);
bool itp_isEndOfCommand(struct Interpreter *interpreter);
enum ErrorCode itp_endOfCommand(struct Interpreter *interpreter);

#endif /* interpreter_h */
