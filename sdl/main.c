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
#include "dev_mode.h"
#include "settings.h"
#include "boot_intro.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <SDL2/SDL.h>
#elif defined(__APPLE__) && defined(__MACH__)
#include <SDL2/SDL.h>
#elif defined(__LINUX__)
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

const char *defaultDisk = "Disk.nx";
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
void loadMainProgram(const char *filename);
void update(void *arg);
void updateScreenRect(int winW, int winH);
void configureJoysticks(void);
void closeJoysticks(void);
void setTouchPosition(int windowX, int windowY);
void getDiskFilename(char *outputString);
void audioCallback(void *userdata, Uint8 *stream, int len);

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
SDL_AudioDeviceID audioDevice = 0;
SDL_AudioSpec audioSpec;

struct Core *core = NULL;
struct DevMode devMode;
struct Settings settings;
struct CoreInput coreInput;

int numJoysticks = 0;
SDL_Joystick *joysticks[2] = {NULL, NULL};
SDL_Rect screenRect;
bool quit = false;
bool releasedTouch = false;
bool audioStarted = false;
Uint32 lastTicks = 0;

int main(int argc, const char * argv[])
{
    memset(&devMode, 0, sizeof(struct DevMode));
    memset(&settings, 0, sizeof(struct Settings));
    memset(&coreInput, 0, sizeof(struct CoreInput));
    
    settings_init(&settings, argc, argv);
    
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_DROPFILE: {
                settings.program = event.drop.file;
                break;
            }
        }
    }
    
    Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    if (settings.fullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    const char *windowTitle = "LowRes NX " CORE_VERSION;
//    const char *windowTitle = "LowRes NX"
    
    window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * defaultWindowScale, SCREEN_HEIGHT * defaultWindowScale, windowFlags);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    configureJoysticks();
    
    core = calloc(1, sizeof(struct Core));
    if (core)
    {
        SDL_AudioSpec desiredAudioSpec = {
            .freq = 44100,
            .format = AUDIO_S16,
            .channels = 2,
            .samples = 735,
            .userdata = core,
            .callback = audioCallback
        };
        
        audioDevice = SDL_OpenAudioDevice(NULL, 0, &desiredAudioSpec, &audioSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        
        struct CoreDelegate coreDelegate;
        memset(&coreDelegate, 0, sizeof(struct CoreDelegate));
        
        core_init(core);
        
        coreDelegate.interpreterDidFail = interpreterDidFail;
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess;
        coreDelegate.diskDriveDidSave = diskDriveDidSave;
        coreDelegate.controlsDidChange = controlsDidChange;
        
        core_setDelegate(core, &coreDelegate);
        
        devMode.core = core;
        devMode.settings = &settings;
        
        if (settings.program)
        {
            loadMainProgram(settings.program);
        }
        else
        {
            loadBootIntro();
#ifdef __EMSCRIPTEN__
            const char *url = "LowResGalaxy2.nx";
            emscripten_async_wget(url, "program.nx", onloaded, onerror);
#endif
        }
        
        updateScreenRect(SCREEN_WIDTH * defaultWindowScale, SCREEN_HEIGHT * defaultWindowScale);
        
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(update, NULL, -1, true);
#else
        while (!quit)
        {
            update(NULL);
        }
#endif
        
        SDL_CloseAudioDevice(audioDevice);
        
        core_deinit(core);
        
        free(core);
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
    devMode.state = DevModeStateOff;
    devMode.mainProgramFilename[0] = 0;
    
    struct CoreError error = core_compileProgram(core, bootIntroSourceCode);
    if (error.code != ErrorNone)
    {
        core_traceError(core, error);
    }
    
    core->interpreter->debug = false;
    core_willRunProgram(core, SDL_GetTicks() / 1000);
}

void loadMainProgram(const char *filename)
{
    struct CoreError error = err_noCoreError();
    devMode.state = DevModeStateOff;
    strncpy(devMode.mainProgramFilename, filename, FILENAME_MAX - 1);
    
    FILE *file = fopen(filename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char *sourceCode = calloc(1, size + 1); // +1 for NULL terminator
        if (sourceCode)
        {
            fread(sourceCode, size, 1, file);
            
            error = core_compileProgram(core, sourceCode);
            free(sourceCode);
        }
        else
        {
            error = err_makeCoreError(ErrorOutOfMemory, -1);
        }
        
        fclose(file);
    }
    else
    {
        error = err_makeCoreError(ErrorCouldNotOpenProgram, -1);
    }
    
    devMode.lastError = error;
    if (error.code != ErrorNone)
    {
        dev_show(&devMode);
    }
    else
    {
        core_willRunProgram(core, SDL_GetTicks() / 1000);
    }
}

void update(void *arg) {
    SDL_Event event;
    
    // limit to 60 FPS
    Uint32 ticks = SDL_GetTicks();
    Uint32 ticksDelta = ticks - lastTicks;
    if (ticksDelta < 16)
    {
        SDL_Delay(16 - ticksDelta);
    }
    lastTicks = ticks;
    
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
                        updateScreenRect(event.window.data1, event.window.data2);
                        break;
                    }
                }
                break;
            
            case SDL_DROPFILE: {
                loadMainProgram(event.drop.file);
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
                
                // console buttons
                if (code == SDLK_RETURN || code == SDLK_p)
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
                    else if (code == SDLK_m)
                    {
                        if (devMode.state != DevModeStateVisible && dev_hasProgram(&devMode))
                        {
                            dev_show(&devMode);
                        }
                    }
                    else if (code == SDLK_r)
                    {
                        if (dev_hasProgram(&devMode))
                        {
                            dev_runProgram(&devMode);
                        }
                    }
                    else if (code == SDLK_e)
                    {
                        loadBootIntro();
                    }
                }
                else if (code == SDLK_ESCAPE)
                {
                    quit = true;
                }
                break;
            }
                
            case SDL_TEXTINPUT: {
                char key = event.text.text[0];
                if (key >= ' ' && key <= '_')
                {
                    coreInput.key = key;
                }
                else if (key >= 'a' && key <= 'z')
                {
                    coreInput.key = key - 32;
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
    
    if (devMode.state == DevModeStateVisible)
    {
        dev_update(&devMode, &coreInput);
        if (devMode.state == DevModeStateOff)
        {
            loadBootIntro();
        }
    }
    else
    {
        core_update(core, &coreInput);
    }
    
    if (!audioStarted && audioDevice)
    {
        audioStarted = true;
        SDL_PauseAudioDevice(audioDevice, 0);
    }
    
    SDL_RenderClear(renderer);
    
    void *pixels = NULL;
    int pitch = 0;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    
    video_renderScreen(core, pixels);
    
    SDL_UnlockTexture(texture);
    SDL_RenderCopy(renderer, texture, NULL, &screenRect);
    
    SDL_RenderPresent(renderer);
}

void updateScreenRect(int winW, int winH)
{
    int factor = fmax(1, fmin(winW / SCREEN_WIDTH, winH / SCREEN_HEIGHT));
    int nxScreenW = SCREEN_WIDTH * factor;
    int nxScreenH = SCREEN_HEIGHT * factor;
    screenRect.x = (winW - nxScreenW) / 2;
    screenRect.y = (winH - nxScreenH) / 2;
    screenRect.w = nxScreenW;
    screenRect.h = nxScreenH;
    SDL_SetTextInputRect(&screenRect);
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

void getDiskFilename(char *outputString)
{
    if (devMode.state == DevModeStateRunningTool)
    {
        strncpy(outputString, devMode.mainProgramFilename, FILENAME_MAX - 1);
    }
    else
    {
        strncpy(outputString, settings.programsPath, FILENAME_MAX - 1);
        strncat(outputString, defaultDisk, FILENAME_MAX - 1);
    }
}

void audioCallback(void *userdata, Uint8 *stream, int len)
{
    int16_t *samples = (int16_t *)stream;
    int numSamples = len / 2;
    audio_renderAudio(userdata, samples, numSamples, audioSpec.freq);
}

/** Called on error */
void interpreterDidFail(void *context, struct CoreError coreError)
{
    core_traceError(core, coreError);
}

/** Returns true if the disk is ready, false if not. In case of not, core_diskLoaded must be called when ready. */
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager)
{
    char diskFilename[FILENAME_MAX];
    getDiskFilename(diskFilename);
    
    FILE *file = fopen(diskFilename, "rb");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        char *sourceCode = calloc(1, size + 1); // +1 for NULL terminator
        if (sourceCode)
        {
            fread(sourceCode, size, 1, file);
            
            struct CoreError error = data_import(diskDataManager, sourceCode, true);
            free(sourceCode);
            
            if (error.code != ErrorNone)
            {
                core_traceError(core, error);
            }
        }
        else
        {
            SDL_Log("not enough memory");
        }
        
        fclose(file);
    }
    
    return true;
}

/** Called when a disk data entry was saved */
void diskDriveDidSave(void *context, struct DataManager *diskDataManager)
{
    char *output = data_export(diskDataManager);
    if (output)
    {
        char diskFilename[FILENAME_MAX];
        getDiskFilename(diskFilename);
        
        FILE *file = fopen(diskFilename, "wb");
        if (file)
        {
            fwrite(output, 1, strlen(output), file);
            fclose(file);
        }
        else
        {
            struct TextLib *lib = &core->overlay->textLib;
            txtlib_printText(lib, "COULD NOT SAVE:\n");
            txtlib_printText(lib, diskFilename);
            txtlib_printText(lib, "\n");
        }

        free(output);
    }
}

/** Called when keyboard or gamepad settings changed */
void controlsDidChange(void *context, struct ControlsInfo controlsInfo)
{
    if (controlsInfo.isKeyboardEnabled)
    {
        if (!SDL_IsTextInputActive())
        {
            SDL_StartTextInput();
        }
    }
    else if (SDL_IsTextInputActive())
    {
        SDL_StopTextInput();
    }
}

#ifdef __EMSCRIPTEN__

void onloaded(const char *filename)
{
    SDL_Log("loaded %s", filename);
    loadMainProgram(filename);
}

void onerror(const char *filename)
{
    SDL_Log("failed to load %s", filename);
}

#endif
