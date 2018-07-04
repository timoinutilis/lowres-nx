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

#include <stdbool.h>
#include <math.h>
#include <string.h>
#include "core.h"
#include "boot_intro.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL2/SDL.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

const int defaultWindowScale = 4;
const int joyAxisThreshold = 16384;

const int keyboardControls[2][8] = {
    // up, down, left, right, button A, button B, alt. button A, alt. button B
    {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_N, SDL_SCANCODE_M},
    {SDL_SCANCODE_E, SDL_SCANCODE_D, SDL_SCANCODE_S, SDL_SCANCODE_F,
        SDL_SCANCODE_TAB, SDL_SCANCODE_Q, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_A}
};

void loadBootIntro(void);
void loadProgram(const char *filename);
void update(void *arg);
void configureJoysticks(void);
void closeJoysticks(void);
void setTouchPosition(int windowX, int windowY);

void interpreterDidFail(void *context, struct CoreError coreError);
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager);
void diskDriveDidSave(void *context, struct DataManager *diskDataManager);
void controlsDidChange(void *context, struct ControlsInfo controlsInfo);

#ifdef __EMSCRIPTEN__
void onloaded(const char *filename);
void onerror(const char *filename);
#endif


SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;

struct Core *core = NULL;
int numJoysticks = 0;
SDL_Joystick *joysticks[2] = {NULL, NULL};
SDL_Rect screenRect;
struct CoreInput coreInput;
bool quit = false;
bool releasedTouch = false;

int main(int argc, const char * argv[])
{
    const char *programArg = NULL;
    bool fullscreenArg = false;
    
    for (int i = 1; i < argc; i++)
    {
        const char *arg = argv[i];
        if (*arg == '-') {
            i++;
            if (i < argc)
            {
                const char *value = argv[i];
                if (strcmp(arg, "-fullscreen") == 0) {
                    if (strcmp(value, "yes") == 0)
                    {
                        fullscreenArg = true;
                    }
                }
            }
            else
            {
                SDL_Log("missing value for parameter %s", arg);
            }
        } else {
            programArg = arg;
        }
    }
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);
    
    Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (fullscreenArg)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    window = SDL_CreateWindow("LowRes NX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * defaultWindowScale, SCREEN_HEIGHT * defaultWindowScale, windowFlags);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    
    configureJoysticks();
    
    core = SDL_calloc(1, sizeof(struct Core));
    if (core)
    {
        SDL_memset(&coreInput, 0, sizeof(struct CoreInput));
        
        struct CoreDelegate coreDelegate;
        SDL_memset(&coreDelegate, 0, sizeof(struct CoreDelegate));
        
        core_init(core);
        
        coreDelegate.interpreterDidFail = interpreterDidFail;
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess;
        coreDelegate.diskDriveDidSave = diskDriveDidSave;
        coreDelegate.controlsDidChange = controlsDidChange;
        
        core_setDelegate(core, &coreDelegate);
        
        if (programArg)
        {
            loadProgram(programArg);
        }
        else
        {
            loadBootIntro();
#ifdef __EMSCRIPTEN__
            const char *url = "LowResGalaxy2.nx";
            emscripten_async_wget(url, "program.nx", onloaded, onerror);
#endif
        }
        
        screenRect.x = 0;
        screenRect.y = 0;
        screenRect.w = SCREEN_WIDTH * defaultWindowScale;
        screenRect.h = SCREEN_HEIGHT * defaultWindowScale;
        
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(update, NULL, -1, true);
#else
        while (!quit)
        {
            update(NULL);
        }
#endif
        
        core_deinit(core);
        
        SDL_free(core);
        core = NULL;
    }
    
    closeJoysticks();
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();
    
    return 0;
}

void loadBootIntro()
{
    struct CoreError error = core_compileProgram(core, bootIntroSourceCode);
    if (error.code != ErrorNone)
    {
        core_traceError(core, error);
    }
    
    core_willRunProgram(core, SDL_GetTicks() / 1000);
}

void loadProgram(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *sourceCode = SDL_calloc(1, size + 1); // +1 for NULL terminator
        if (sourceCode)
        {
            fread(sourceCode, size, 1, file);
            
            struct CoreError error = core_compileProgram(core, sourceCode);
            SDL_free(sourceCode);
            
            if (error.code != ErrorNone)
            {
                core_traceError(core, error);
            }
            
            core_willRunProgram(core, SDL_GetTicks() / 1000);
        }
        else
        {
            SDL_Log("not enough memory");
        }
        
        fclose(file);
    }
    else
    {
        SDL_Log("failed to load file: %s", filename);
    }
}

void update(void *arg) {
    SDL_Event event;
    
    if (releasedTouch)
    {
        coreInput.touch = false;
        releasedTouch = false;
    }
    
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
                if (event.key.keysym.mod & KMOD_CTRL)
                {
                    if (code == SDLK_d)
                    {
                        core_setDebug(core, !core_getDebug(core));
                    }
                    else if (code == SDLK_f)
                    {
                        if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
                        {
                            SDL_SetWindowFullscreen(window, 0);
                        }
                        else
                        {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                    }
                }
                else if (code == SDLK_ESCAPE)
                {
                    quit = true;
                }
                break;
            }
                
            case SDL_MOUSEBUTTONDOWN: {
                setTouchPosition(event.button.x, event.button.y);
                coreInput.touch = true;
                break;
            }
                
            case SDL_MOUSEBUTTONUP: {
                releasedTouch = true;
                break;
            }
                
            case SDL_MOUSEMOTION: {
                setTouchPosition(event.motion.x, event.motion.y);
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
            Sint16 axisX = SDL_JoystickGetAxis(joy, 0);
            Sint16 axisY = SDL_JoystickGetAxis(joy, 1);
            gamepad->up = (hat & SDL_HAT_UP) != 0 || axisY < -joyAxisThreshold;
            gamepad->down = (hat & SDL_HAT_DOWN) != 0 || axisY > joyAxisThreshold;
            gamepad->left = (hat & SDL_HAT_LEFT) != 0 || axisX < -joyAxisThreshold;
            gamepad->right = (hat & SDL_HAT_RIGHT) != 0 || axisX > joyAxisThreshold;
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

void setTouchPosition(int windowX, int windowY)
{
    coreInput.touchX = (windowX - screenRect.x) * SCREEN_WIDTH / screenRect.w;
    coreInput.touchY = (windowY - screenRect.y) * SCREEN_HEIGHT / screenRect.h;
}

/** Called on error */
void interpreterDidFail(void *context, struct CoreError coreError)
{
    core_traceError(core, coreError);
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

#ifdef __EMSCRIPTEN__

void onloaded(const char *filename)
{
    SDL_Log("loaded %s", filename);
    loadProgram(filename);
}

void onerror(const char *filename)
{
    SDL_Log("failed to load %s", filename);
}

#endif
