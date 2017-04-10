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

#ifndef big_endian_h
#define big_endian_h

#include <stdio.h>

typedef struct {
    uint8_t hi;
    uint8_t lo;
} BigEndianUInt16;

void BigEndianUInt16_set(BigEndianUInt16 *variable, int value);
int BigEndianUInt16_get(BigEndianUInt16 *variable);

#endif /* big_endian_h */
