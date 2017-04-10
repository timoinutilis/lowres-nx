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

#include "big_endian.h"

void BigEndianUInt16_set(BigEndianUInt16 *variable, int value)
{
    variable->hi = (value & 0xFF00) >> 8;
    variable->lo = value & 0xFF;
}

int BigEndianUInt16_get(BigEndianUInt16 *variable)
{
    return (variable->hi << 8) | variable->lo;
}
