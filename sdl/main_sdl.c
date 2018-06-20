//
//  main_sdl.c
//  LowRes NX macOS SDL
//
//  Created by Timo Kloss on 18/6/18.
//  Copyright © 2018 Inutilis Software. All rights reserved.
//

#include "main_sdl.h"
#include <stdbool.h>
#include <math.h>
#include <SDL2/SDL.h>
#include "core.h"

#define DEFAULT_WINDOW_SCALE 4

void loadProgram(const char *filename);
void configureJoysticks(void);
void closeJoysticks(void);

void interpreterDidFail(void *context, struct CoreError coreError);
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager);
void diskDriveDidSave(void *context, struct DataManager *diskDataManager);
void controlsDidChange(void *context, struct ControlsInfo controlsInfo);

struct Core *core = NULL;
char *sourceCode = NULL;
int numJoysticks = 0;
SDL_Joystick *joysticks[2] = {NULL, NULL};

int keyboardControls[2][8] = {
    // up, down, left, right, button A, button B, alt. button A, alt. button B
    {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_COMMA, SDL_SCANCODE_PERIOD},
    {SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_D, SDL_SCANCODE_G, SDL_SCANCODE_A, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_S}
};

int mainSDL(int argc, const char * argv[])
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    
    SDL_Window *window = SDL_CreateWindow("LowRes NX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * DEFAULT_WINDOW_SCALE, SCREEN_HEIGHT * DEFAULT_WINDOW_SCALE, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    
    configureJoysticks();
    
    core = SDL_calloc(1, sizeof(struct Core));
    if (core)
    {
        struct CoreInput coreInput;
        SDL_memset(&coreInput, 0, sizeof(struct CoreInput));
        
        struct CoreDelegate coreDelegate;
        SDL_memset(&coreDelegate, 0, sizeof(struct CoreDelegate));
        
        core_init(core);
        
        coreDelegate.interpreterDidFail = interpreterDidFail;
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess;
        coreDelegate.diskDriveDidSave = diskDriveDidSave;
        coreDelegate.controlsDidChange = controlsDidChange;
        
        core_setDelegate(core, &coreDelegate);
        
//        loadProgram("/Users/timokloss/projects/lowres-nx/programs/Star Scroller 1.0.nx");
        
        SDL_Rect screenRect;
        screenRect.x = 0;
        screenRect.y = 0;
        screenRect.w = SCREEN_WIDTH * DEFAULT_WINDOW_SCALE;
        screenRect.h = SCREEN_HEIGHT * DEFAULT_WINDOW_SCALE;
        
        bool quit = false;
        while (!quit)
        {
            SDL_Event event;
            
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_QUIT:
                        quit = true;
                        break;
                        
                    case SDL_WINDOWEVENT:
                        switch (event.window.event)
                        {
                            case SDL_WINDOWEVENT_RESIZED: {
                                int winW = event.window.data1;
                                int winH = event.window.data2;
                                int factor = fmax(1, fmin(winW / SCREEN_WIDTH, winH / SCREEN_HEIGHT));
                                int nxScreenW = SCREEN_WIDTH * factor;
                                int nxScreenH = SCREEN_HEIGHT * factor;
                                screenRect.x = (winW - nxScreenW) / 2;
                                screenRect.y = (winH - nxScreenH) / 2;
                                screenRect.w = nxScreenW;
                                screenRect.h = nxScreenH;
                                break;
                            }
                        }
                        break;
                        
                    case SDL_DROPFILE: {
                        loadProgram(event.drop.file);
                        break;
                    }
                        
                    case SDL_KEYDOWN: {
                        SDL_Keycode code = event.key.keysym.sym;
                        
                        // text input
                        if (code == SDLK_RETURN)
                        {
                            coreInput.key = CoreInputKeyReturn;
                        }
                        else if (code == SDLK_BACKSPACE)
                        {
                            coreInput.key = CoreInputKeyBackspace;
                        }
                        else if (code >= SDLK_SPACE && code <= SDLK_UNDERSCORE)
                        {
                            coreInput.key = code;
                        }
                        else if (code >= SDLK_a && code <= SDLK_z)
                        {
                            coreInput.key = code - 32;
                        }
                        
                        // console buttons
                        if (code == SDLK_p)
                        {
                            coreInput.pause = true;
                        }
                        
                        // system
                        if (event.key.keysym.mod & KMOD_GUI)
                        {
                            if (code == SDLK_d)
                            {
                                core_setDebug(core, !core_getDebug(core));
                            }
                        }
                        break;
                    }
                        
                    case SDL_MOUSEBUTTONDOWN: {
                        coreInput.touch = true;
                        break;
                    }
                        
                    case SDL_MOUSEBUTTONUP: {
                        coreInput.touch = false;
                        break;
                    }
                    
                    case SDL_MOUSEMOTION: {
                        int x = (event.motion.x - screenRect.x) * SCREEN_WIDTH / screenRect.w;
                        int y = (event.motion.y - screenRect.y) * SCREEN_HEIGHT / screenRect.h;
                        coreInput.touchX = x;
                        coreInput.touchY = y;
                        break;
                    }
                        
                    case SDL_JOYDEVICEADDED:
                    case SDL_JOYDEVICEREMOVED: {
                        configureJoysticks();
                        break;
                    }
                        
                    case SDL_JOYBUTTONDOWN: {
                        if (event.jbutton.button == 2)
                        {
                            coreInput.pause = true;
                        }
                        break;
                    }
                }
            }
            
            const Uint8 *state = SDL_GetKeyboardState(NULL);
            for (int i = 0; i < 2; i++)
            {
                struct CoreInputGamepad *gamepad = &coreInput.gamepads[i];
                if (i < numJoysticks)
                {
                    SDL_Joystick *joy = joysticks[i];
                    Uint8 hat = SDL_JoystickGetHat(joy, 0);
                    gamepad->up = (hat & SDL_HAT_UP) != 0;
                    gamepad->down = (hat & SDL_HAT_DOWN) != 0;
                    gamepad->left = (hat & SDL_HAT_LEFT) != 0;
                    gamepad->right = (hat & SDL_HAT_RIGHT) != 0;
                    gamepad->buttonA = SDL_JoystickGetButton(joy, 0);
                    gamepad->buttonB = SDL_JoystickGetButton(joy, 1);
                }
                else
                {
                    int ci = i - numJoysticks;
                    gamepad->up = state[keyboardControls[ci][0]];
                    gamepad->down = state[keyboardControls[ci][1]];
                    gamepad->left = state[keyboardControls[ci][2]];
                    gamepad->right = state[keyboardControls[ci][3]];
                    gamepad->buttonA = state[keyboardControls[ci][4]] || state[keyboardControls[ci][6]];
                    gamepad->buttonB = state[keyboardControls[ci][5]] || state[keyboardControls[ci][7]];
                }
            }
            
            core_update(core, &coreInput);
            
            SDL_RenderClear(renderer);
            
            void *pixels = NULL;
            int pitch = 0;
            SDL_LockTexture(texture, NULL, &pixels, &pitch);
            
            video_renderScreen(core, pixels, pitch);
            
            SDL_UnlockTexture(texture);
            SDL_RenderCopy(renderer, texture, NULL, &screenRect);
            
            SDL_RenderPresent(renderer);
        }
        
        core_deinit(core);
        
        SDL_free(core);
        core = NULL;
        
        SDL_free(sourceCode);
        sourceCode = NULL;
    }
    
    closeJoysticks();
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();
    
    return 0;
}

void loadProgram(const char *filename)
{
    SDL_free(sourceCode);
    sourceCode = NULL;
    
    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        sourceCode = SDL_calloc(1, size + 1); // +1 for NULL terminator
        if (sourceCode)
        {
            fread(sourceCode, size, 1, file);
        }
        fclose(file);

        if (sourceCode)
        {
            struct CoreError error = core_compileProgram(core, sourceCode);
            SDL_Log("compile error: %d", error.code);
            
            core_willRunProgram(core, SDL_GetTicks() / 1000);
        }
        else
        {
            SDL_Log("not enough memory");
        }
    }
    else
    {
        SDL_Log("failed to load file: %s", filename);
    }
}

void configureJoysticks() {
    closeJoysticks();
    numJoysticks = SDL_NumJoysticks();
    if (numJoysticks > 2)
    {
        numJoysticks = 2;
    }
    for (int i = 0; i < numJoysticks; i++)
    {
        joysticks[i] = SDL_JoystickOpen(i);
    }
}

void closeJoysticks() {
    for (int i = 0; i < numJoysticks; i++)
    {
        SDL_JoystickClose(joysticks[i]);
        joysticks[i] = NULL;
    }
    numJoysticks = 0;
}

/** Called on error */
void interpreterDidFail(void *context, struct CoreError coreError)
{
    
}

/** Returns true if the disk is ready, false if not. In case of not, core_diskLoaded must be called when ready. */
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager)
{
    return true;
}

/** Called when a disk data entry was saved */
void diskDriveDidSave(void *context, struct DataManager *diskDataManager)
{
    
}

/** Called when keyboard or gamepad settings changed */
void controlsDidChange(void *context, struct ControlsInfo controlsInfo)
{
    
}
