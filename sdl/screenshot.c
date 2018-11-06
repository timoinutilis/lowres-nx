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
#include "system_paths.h"
#include "core.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

bool writeImage(const char *filename, int width, int height, uint32_t *pixels, int scale)
{
    uint8_t *data = malloc(width * height * 3 * scale * scale);
    if (data)
    {
        int i = 0;
        for (int y = 0; y < height; y++)
        {
            for (int ys = 0; ys < scale; ys++)
            {
                for (int x = 0; x < width; x++)
                {
                    uint32_t pixel = pixels[y * width + x];
                    for (int xs = 0; xs < scale; xs++)
                    {
                        data[i++] = (pixel) & 0xFF;
                        data[i++] = (pixel >> 8) & 0xFF;
                        data[i++] = (pixel >> 16) & 0xFF;
                    }
                }
            }
        }
        
        int result = stbi_write_png(filename, width * scale, height * scale, 3, data, width * 3 * scale);
        free(data);
        
        return (result != 0);
    }
    return false;
}

bool screenshot_save(uint32_t *pixels, int scale)
{
    char filename[FILENAME_MAX];
    
    desktop_path(filename, FILENAME_MAX);
    size_t len = strlen(filename);
    
    time_t rawtime;
    time(&rawtime);
    struct tm *timeinfo = localtime(&rawtime);
    strftime(&filename[len], FILENAME_MAX - len - 1, "LowRes NX %Y-%m-%d %H_%M_%S.png", timeinfo);

    return writeImage(filename, SCREEN_WIDTH, SCREEN_HEIGHT, pixels, scale);
}

#endif
