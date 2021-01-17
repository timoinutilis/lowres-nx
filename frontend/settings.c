//
// Copyright 2017-2020 Timo Kloss
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

#include "config.h"

#include "settings.h"
#include "system_paths.h"
#include "utils.h"
#ifndef __LIBRETRO__
#include "sdl_include.h"
#endif
#include <string.h>

const char *optionYes = "yes";
const char *optionNo = "no";

bool settings_filename(char *destination);
void settings_setParameter(struct Parameters *parameters, const char *key, const char *value);
void settings_saveAs(struct Settings *settings, const char *filename);

void settings_init(struct Settings *settings, char *filenameOut, int argc, const char * argv[])
{
    memset(settings, 0, sizeof(struct Settings));
    
#if SETTINGS_FILE
    
    // load settings file
    
    char filename[FILENAME_MAX];
    if (settings_filename(filename))
    {
        FILE *file = fopen_utf8(filename, "r");
        if (file)
        {
            char line[FILENAME_MAX];
            while (fgets(line, FILENAME_MAX, file))
            {
                if (line[0] != '#')
                {
                    char *space = strchr(line, ' ');
                    if (space)
                    {
                        *space = 0; // separate into two strings
                        char *value = space + 1;
                        
                        // remove EOL characters
                        char *eolChar = strchr(value, '\n');
                        if (eolChar)
                        {
                            *eolChar = 0;
                        }
                        eolChar = strchr(value, '\r');
                        if (eolChar)
                        {
                            *eolChar = 0;
                        }
                        
                        if (strcmp(line, "tool") == 0)
                        {
                            settings_addTool(settings, value);
                        }
                        else
                        {
                            settings_setParameter(&settings->file, line, value);
                        }
                    }
                }
            }
            
            fclose(file);
        }
        else
        {
            // write default settings file
            settings_saveAs(settings, filename);
        }
        
        // copy file parameters to session parameters
        memcpy(&settings->session, &settings->file, sizeof(struct Parameters));
    }
    
#endif
    
    // parse arguments
    
    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        if (*arg == '-') {
            i++;
            if (i < argc)
            {
                settings_setParameter(&settings->session, arg + 1, argv[i]);
            }
            else
            {
                printf("missing value for parameter %s\n", arg);
            }
        } else {
            strncpy(filenameOut, arg, FILENAME_MAX - 1);
        }
    }
}

bool settings_filename(char *destination)
{
#if SETTINGS_FILE && !__LIBRETRO__
    char *prefPath = SDL_GetPrefPath("Inutilis Software", "LowRes NX");
    if (prefPath)
    {
        strncpy(destination, prefPath, FILENAME_MAX - 1);
        strncat(destination, "settings.txt", FILENAME_MAX - 1);
        SDL_free((void *)prefPath);
        return true;
    }
#endif
    return false;
}

void settings_setParameter(struct Parameters *parameters, const char *key, const char *value)
{
    if (strcmp(key, "fullscreen") == 0) {
        if (strcmp(value, optionYes) == 0)
        {
            parameters->fullscreen = true;
        }
        else if (strcmp(value, optionNo) == 0)
        {
            parameters->fullscreen = false;
        }
    }
    else if (strcmp(key, "disabledev") == 0)
    {
        if (strcmp(value, optionYes) == 0)
        {
            parameters->disabledev = true;
        }
        else if (strcmp(value, optionNo) == 0)
        {
            parameters->disabledev = false;
        }
    }
    else if (strcmp(key, "mapping") == 0)
    {
        int i = atoi(value);
        if (i >= 0 && i <= 1)
        {
            parameters->mapping = i;
        }
    }
    else if (strcmp(key, "disabledelay") == 0)
    {
        if (strcmp(value, optionYes) == 0)
        {
            parameters->disabledelay = true;
        }
        else if (strcmp(value, optionNo) == 0)
        {
            parameters->disabledelay = false;
        }
    }
    else if (strcmp(key, "zoom") == 0)
    {
        int i = atoi(value);
        if (i >= 0 && i <= 3)
        {
            parameters->zoom = i;
        }
    }
    else
    {
        printf("unknown parameter %s\n", key);
    }
}

void settings_save(struct Settings *settings)
{
#if SETTINGS_FILE
    char filename[FILENAME_MAX];
    if (settings_filename(filename))
    {
        settings_saveAs(settings, filename);
    }
#endif
}

void settings_saveAs(struct Settings *settings, const char *filename)
{
#if SETTINGS_FILE
    FILE *file = fopen_utf8(filename, "w");
    if (file)
    {
        fputs("# Start the application in fullscreen mode.\n# fullscreen yes/no\n", file);
        fputs("fullscreen ", file);
        fputs(settings->file.fullscreen ? optionYes : optionNo, file);
        fputs("\n\n", file);
        
        fputs("# Start the application in zoom mode: 0 = pixel perfect, 1 = large, 2 = overscan, 3 = squeeze.\n# zoom 0-3\n", file);
        fprintf(file, "zoom %d\n\n", settings->file.zoom);
        
        fputs("# Disable the Development Menu, Esc key quits LowRes NX.\n# disabledev yes/no\n", file);
        fputs("disabledev ", file);
        fputs(settings->file.disabledev ? optionYes : optionNo, file);
        fputs("\n\n", file);
        
        fputs("# Set the key mapping. 0 = standard, 1 = GameShell.\n# mapping 0-1\n", file);
        fprintf(file, "mapping %d\n\n", settings->file.mapping);
        
        fputs("# Disable the delay for too short frames.\n# disabledelay yes/no\n", file);
        fputs("disabledelay ", file);
        fputs(settings->file.disabledelay ? optionYes : optionNo, file);
        fputs("\n\n", file);
        
        fputs("# Add tools for the Edit ROM menu (max 4).\n# tool My Tool.nx\n", file);
        for (int i = 0; i < settings->numTools; i++)
        {
            fputs("tool ", file);
            fputs(settings->tools[i], file);
            fputs("\n", file);
        }
        
        fclose(file);
    }
#endif
}

bool settings_addTool(struct Settings *settings, const char *filename)
{
    int index = settings->numTools;
    if (index < MAX_TOOLS)
    {
        strncpy(settings->tools[index], filename, FILENAME_MAX - 1);
        displayName(filename, settings->toolNames[index], TOOL_NAME_SIZE);
        settings->numTools++;
        return true;
    }
    return false;
}

void settings_removeTool(struct Settings *settings, int index)
{
    if (index < settings->numTools)
    {
        for (int i = index; i < MAX_TOOLS - 1; i++)
        {
            strncpy(settings->tools[i], settings->tools[i + 1], FILENAME_MAX - 1);
            strncpy(settings->toolNames[i], settings->toolNames[i + 1], TOOL_NAME_SIZE - 1);
        }
        settings->tools[MAX_TOOLS - 1][0] = 0;
        settings->toolNames[MAX_TOOLS - 1][0] = 0;
        settings->numTools--;
    }
}
