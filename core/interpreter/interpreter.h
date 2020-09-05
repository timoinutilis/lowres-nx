//
// Copyright 2016-2017 Timo Kloss
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
enum ErrorCode itp_labelStackError(struct LabelStackItem *item);

#endif /* interpreter_h */
