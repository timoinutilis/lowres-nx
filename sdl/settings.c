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

#include "config.h"

#include "settings.h"
#include "system_paths.h"
#include "utils.h"
#include "sdl_include.h"
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
        FILE *file = fopen(filename, "r");
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
#if SETTINGS_FILE
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
    FILE *file = fopen(filename, "w");
    if (file)
    {
        fputs("# Start the application in fullscreen mode.\n# fullscreen yes/no\n", file);
        fputs("fullscreen ", file);
        fputs(settings->file.fullscreen ? optionYes : optionNo, file);
        fputs("\n\n", file);
        
        fputs("# Disable the Development Menu, ESC key quits LowRes NX.\n# disabledev yes/no\n", file);
        fputs("disabledev ", file);
        fputs(settings->file.disabledev ? optionYes : optionNo, file);
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
