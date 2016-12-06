//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes Core.
//
// LowRes Core is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes Core is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes Core.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef lowrescore_video_h
#define lowrescore_video_h

typedef struct character {
    uint8_t data[16];
} character_t;

typedef struct character_bank {
    character_t characters[256];
} character_bank_t;

typedef struct palette {
    uint8_t colors[4];
} palette_t;

typedef struct sprite {
    uint8_t x;
    uint8_t y;
    uint8_t character;
    uint8_t attributes; // bank, palette, flip, priority, size, zoom
} sprite_t;

typedef struct cell {
    uint8_t character;
    uint8_t attributes; // bank, palette, flip, priority
} cell_t;

typedef struct layer {
    uint8_t x;
    uint8_t y;
    uint8_t scroll_x;
    uint8_t scroll_y;
    uint8_t attributes; // visible, bank,
    cell_t cells[32][32];
} layer_t;

typedef struct core_video {
    character_bank_t character_banks[2];
    palette_t palettes[8];
    layer_t layers[2];
    sprite_t sprites[64];
} core_video_t;

#endif /* lowrescore_video_h */
