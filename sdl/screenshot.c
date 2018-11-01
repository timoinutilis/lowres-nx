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

#include "config.h"

#if SCREENSHOTS

#include "screenshot.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <png.h>
#include "system_paths.h"
#include "core.h"

int writeImage(const char *filename, int width, int height, uint32_t *pixels, int scale, char *title)
{
    int code = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;
    
    // Open file for writing (binary mode)
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        code = 1;
        goto finalise;
    }
    
    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fprintf(stderr, "Could not allocate write struct\n");
        code = 1;
        goto finalise;
    }
    
    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr, "Could not allocate info struct\n");
        code = 1;
        goto finalise;
    }
    
    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during png creation\n");
        code = 1;
        goto finalise;
    }
    
    png_init_io(png_ptr, fp);
    
    // Write header (8 bit colour depth)
    png_set_IHDR(png_ptr, info_ptr, width * scale, height * scale,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    
    // Set title
    if (title != NULL) {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }
    
    png_write_info(png_ptr, info_ptr);
    
    // Allocate memory for one row (3 bytes per pixel - RGB)
    row = (png_bytep) malloc(3 * width * scale * sizeof(png_byte));
    
    // Write image data
    int x, y, i, s;
    for (y = 0; y < height; y++)
    {
        i = 0;
        for (x = 0; x < width; x++)
        {
            uint32_t pixel = pixels[y * width + x];
            for (s = 0; s < scale; s++)
            {
                row[i++] = (pixel) & 0xFF;
                row[i++] = (pixel >> 8) & 0xFF;
                row[i++] = (pixel >> 16) & 0xFF;
            }
        }
        for (s = 0; s < scale; s++)
        {
            png_write_row(png_ptr, row);
        }
    }
    
    // End write
    png_write_end(png_ptr, NULL);
    
finalise:
    if (fp != NULL) fclose(fp);
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL) free(row);
    
    return code;
}

void screenshot_save(uint32_t *pixels, int scale)
{
    char filename[FILENAME_MAX];
    
    desktop_path(filename, FILENAME_MAX);
    size_t len = strlen(filename);
    
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(&filename[len], FILENAME_MAX - len - 1, "LowRes NX %Y-%m-%d %H_%M_%S.png", timeinfo);

    writeImage(filename, SCREEN_WIDTH, SCREEN_HEIGHT, pixels, scale, "LowRes NX Screenshot");
}

#endif
