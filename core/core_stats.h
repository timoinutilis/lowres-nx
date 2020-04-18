//
// Copyright 2016-2020 Timo Kloss
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

#ifndef core_stats_h
#define core_stats_h

#include <stdio.h>
#include "error.h"
#include "tokenizer.h"
#include "data_manager.h"

struct Stats {
    struct Tokenizer *tokenizer;
    struct DataManager *romDataManager;
    int numTokens;
    int romSize;
};

void stats_init(struct Stats *stats);
void stats_deinit(struct Stats *stats);
struct CoreError stats_update(struct Stats *stats, const char *sourceCode);

#endif /* core_stats_h */
