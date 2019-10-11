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

#ifndef error_h
#define error_h

#include <stdio.h>

enum ErrorCode {
    ErrorNone,
    
    ErrorCouldNotOpenProgram,
    ErrorTooManyTokens,
    ErrorRomIsFull,
    ErrorIndexAlreadyDefined,
    ErrorUnterminatedString,
    ErrorUnexpectedCharacter,
    ErrorReservedKeyword,
    ErrorSyntax,
    ErrorSymbolNameTooLong,
    ErrorTooManySymbols,
    ErrorTypeMismatch,
    ErrorOutOfMemory,
    ErrorElseWithoutIf,
    ErrorEndIfWithoutIf,
    ErrorExpectedCommand,
    ErrorNextWithoutFor,
    ErrorLoopWithoutDo,
    ErrorUntilWithoutRepeat,
    ErrorWendWithoutWhile,
    ErrorLabelAlreadyDefined,
    ErrorTooManyLabels,
    ErrorExpectedLabel,
    ErrorUndefinedLabel,
    ErrorArrayNotDimensionized,
    ErrorArrayAlreadyDimensionized,
    ErrorVariableAlreadyUsed,
    ErrorIndexOutOfBounds,
    ErrorWrongNumberOfDimensions,
    ErrorInvalidParameter,
    ErrorReturnWithoutGosub,
    ErrorStackOverflow,
    ErrorOutOfData,
    ErrorIllegalMemoryAccess,
    ErrorTooManyCPUCyclesInInterrupt,
    ErrorNotAllowedInInterrupt,
    ErrorIfWithoutEndIf,
    ErrorForWithoutNext,
    ErrorDoWithoutLoop,
    ErrorRepeatWithoutUntil,
    ErrorWhileWithoutWend,
    ErrorDirectoryNotLoaded,
    ErrorDivisionByZero,
    ErrorVariableNotInitialized,
    ErrorArrayVariableWithoutIndex,
    ErrorEndSubWithoutSub,
    ErrorSubWithoutEndSub,
    ErrorSubCannotBeNested,
    ErrorUndefinedSubprogram,
    ErrorExpectedSubprogramName,
    ErrorArgumentCountMismatch,
    ErrorSubAlreadyDefined,
    ErrorTooManySubprograms,
    ErrorSharedOutsideOfASubprogram,
    ErrorGlobalInsideOfASubprogram,
    ErrorExitSubOutsideOfASubprogram,
    ErrorKeyboardNotEnabled,
    ErrorAutomaticPauseNotDisabled,
    ErrorGamepadNotEnabled,
    ErrorTouchNotEnabled,
    ErrorInputChangeNotAllowed,
};

struct CoreError {
    enum ErrorCode code;
    int sourcePosition;
};

const char *err_getString(enum ErrorCode errorCode);
struct CoreError err_makeCoreError(enum ErrorCode code, int sourcePosition);
struct CoreError err_noCoreError(void);

#endif /* error_h */
