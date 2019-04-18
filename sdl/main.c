//
// Copyright 2017-2018 Timo Kloss
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

#include "main.h"
#include "core.h"
#include "runner.h"
#include "dev_menu.h"
#include "settings.h"
#include "system_paths.h"
#include "utils.h"
#include "boot_intro.h"
#include "sdl_include.h"

#if SCREENSHOTS
#include "screenshot.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <math.h>
#include <string.h>

const char *defaultDisk = "Disk.nx";
const int defaultWindowScale = 4;
const int joyAxisThreshold = 16384;
const int bootIntroStateAddress = 0xA000;

const int keyboardControls[2][8] = {
    // up, down, left, right, button A, button B, alt. button A, alt. button B
    {SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_Z, SDL_SCANCODE_X, SDL_SCANCODE_N, SDL_SCANCODE_M},
    {SDL_SCANCODE_E, SDL_SCANCODE_D, SDL_SCANCODE_S, SDL_SCANCODE_F,
        SDL_SCANCODE_TAB, SDL_SCANCODE_Q, SDL_SCANCODE_LSHIFT, SDL_SCANCODE_A}
};

void update(void *arg);
void updateScreenRect(int winW, int winH);
void configureJoysticks(void);
void closeJoysticks(void);
void setTouchPosition(int windowX, int windowY);
void audioCallback(void *userdata, Uint8 *stream, int len);
void saveScreenshot(int scale);

#ifdef __EMSCRIPTEN__
void onloaded(const char *filename);
void onerror(const char *filename);
#endif


SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *texture = NULL;
SDL_AudioDeviceID audioDevice = 0;
SDL_AudioSpec audioSpec;

struct Runner runner;
#if DEV_MENU
struct DevMenu devMenu;
#endif
struct Settings settings;
struct CoreInput coreInput;

enum MainState mainState = MainStateUndefined;
char mainProgramFilename[FILENAME_MAX] = "";

int numJoysticks = 0;
SDL_Joystick *joysticks[2] = {NULL, NULL};
SDL_Rect screenRect;
bool quit = false;
bool releasedTouch = false;
bool audioStarted = false;
bool mouseEnabled = false;
int messageNumber = 0;

int main(int argc, const char * argv[])
{
    memset(&coreInput, 0, sizeof(struct CoreInput));
    
    settings_init(&settings, mainProgramFilename, argc, argv);
    runner_init(&runner);
#if DEV_MENU
    dev_init(&devMenu, &runner, &settings);
#endif
    
    if (runner_isOkay(&runner))
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
        
        SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_DROPFILE: {
                    strncpy(mainProgramFilename, event.drop.file, FILENAME_MAX - 1);
                    SDL_free(event.drop.file);
                    break;
                }
            }
        }
        
        Uint32 windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
        if (settings.session.fullscreen)
        {
            windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
        
        const char *windowTitle = "LowRes NX";
        
        window = SDL_CreateWindow(windowTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * defaultWindowScale, SCREEN_HEIGHT * defaultWindowScale, windowFlags);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
        
        SDL_AudioSpec desiredAudioSpec = {
            .freq = 44100,
            .format = AUDIO_S16,
            .channels = NUM_CHANNELS,
#ifdef __EMSCRIPTEN__
            .samples = 2048, // sample FRAMES
#else
            .samples = 1470, // sample FRAMES
#endif
            .userdata = runner.core,
            .callback = audioCallback
        };
        
        audioDevice = SDL_OpenAudioDevice(NULL, 0, &desiredAudioSpec, &audioSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
        
        configureJoysticks();
        
        bootNX();
        if (mainProgramFilename[0] != 0)
        {
            machine_poke(runner.core, bootIntroStateAddress, 1);
        }

        updateScreenRect(SCREEN_WIDTH * defaultWindowScale, SCREEN_HEIGHT * defaultWindowScale);
        
#ifdef __EMSCRIPTEN__
        emscripten_set_main_loop_arg(update, NULL, -1, true);
#else
        while (!quit)
        {
            Uint32 ticks = SDL_GetTicks();
            
            update(NULL);
            
            // limit to 60 FPS
            Uint32 ticksDelta = SDL_GetTicks() - ticks;
            if (ticksDelta < 16)
            {
                SDL_Delay(16 - ticksDelta);
            }
        }
#endif
    }
    
    closeJoysticks();
    
    SDL_CloseAudioDevice(audioDevice);
    
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();
    
    runner_deinit(&runner);
    
    return 0;
}

void bootNX()
{
    mainState = MainStateBootIntro;
    
    struct CoreError error = core_compileProgram(runner.core, bootIntroSourceCode);
    if (error.code != ErrorNone)
    {
        core_traceError(runner.core, error);
    }
    
    runner.core->interpreter->debug = false;
    core_willRunProgram(runner.core, SDL_GetTicks() / 1000);
}

void rebootNX()
{
    mainProgramFilename[0] = 0;
    bootNX();
}

bool hasProgram()
{
    return mainProgramFilename[0] != 0;
}

const char *getMainProgramFilename()
{
    return mainProgramFilename;
}

void selectProgram(const char *filename)
{
    strncpy(mainProgramFilename, filename, FILENAME_MAX - 1);
    if (mainState == MainStateBootIntro)
    {
        machine_poke(runner.core, bootIntroStateAddress, 1);
    }
    else
    {
        runMainProgram();
    }
}

void runMainProgram()
{
    struct CoreError error = runner_loadProgram(&runner, mainProgramFilename);
#if DEV_MENU
    devMenu.lastError = error;
#endif
    if (error.code != ErrorNone)
    {
#if DEV_MENU
        showDevMenu();
#else
        core_traceError(runner.core, error);
#endif
    }
    else
    {
        core_willRunProgram(runner.core, SDL_GetTicks() / 1000);
        mainState = MainStateRunningProgram;
    }
}

void runToolProgram(const char *filename)
{
    struct CoreError error = runner_loadProgram(&runner, filename);
    if (error.code == ErrorNone)
    {
        mainState = MainStateRunningTool;
        runner.core->interpreter->debug = false;
        core_willRunProgram(runner.core, SDL_GetTicks() / 1000);
    }
    else
    {
        core_traceError(runner.core, error);
    }
}

void showDevMenu()
{
#if DEV_MENU
    bool reload = (mainState == MainStateRunningTool);
    mainState = MainStateDevMenu;
    dev_show(&devMenu, reload);
#endif
}

bool usesMainProgramAsDisk()
{
    return (mainState == MainStateRunningTool);
}

void getDiskFilename(char *outputString)
{
    if (usesMainProgramAsDisk())
    {
        strncpy(outputString, mainProgramFilename, FILENAME_MAX - 1);
    }
    else
    {
        strncpy(outputString, mainProgramFilename, FILENAME_MAX - 1);
        char *separator = strrchr(outputString, PATH_SEPARATOR_CHAR);
        if (separator)
        {
            separator++;
            *separator = 0;
            strncat(outputString, defaultDisk, FILENAME_MAX - 1);
        }
        else
        {
            strncpy(outputString, defaultDisk, FILENAME_MAX - 1);
        }
    }
}

void updateMouseMode()
{
    if (!mouseEnabled && (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP))
    {
        SDL_ShowCursor(SDL_DISABLE);
    }
    else
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
}

void setMouseEnabled(bool enabled)
{
    mouseEnabled = enabled;
    updateMouseMode();
}

void update(void *arg)
{
    SDL_Event event;
    bool hasInput = false;
    
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
                if (hasPostfix(event.drop.file, ".nx") || hasPostfix(event.drop.file, ".NX"))
                {
#if DEV_MENU
                    bool handled = (mainState == MainStateDevMenu && dev_handleDropFile(&devMenu, event.drop.file));
                    if (!handled)
                    {
                        selectProgram(event.drop.file);
                    }
#else
                    selectProgram(event.drop.file);
#endif
                }
                else
                {
                    overlay_message(runner.core, "NOT NX FORMAT");
                }
                SDL_free(event.drop.file);
                break;
            }
            
            case SDL_KEYDOWN: {
                SDL_Keycode keycode = event.key.keysym.sym;
                SDL_Scancode scancode = event.key.keysym.scancode;
                
                if (event.key.keysym.mod == 0)
                {
                    hasInput = true;
                }
                
                // text input
                if (keycode == SDLK_RETURN)
                {
                    coreInput.key = CoreInputKeyReturn;
                }
                else if (keycode == SDLK_BACKSPACE)
                {
                    coreInput.key = CoreInputKeyBackspace;
                }
                else if (scancode == SDL_SCANCODE_UP)
                {
                    coreInput.key = CoreInputKeyUp;
                }
                else if (scancode == SDL_SCANCODE_DOWN)
                {
                    coreInput.key = CoreInputKeyDown;
                }
                else if (scancode == SDL_SCANCODE_LEFT)
                {
                    coreInput.key = CoreInputKeyLeft;
                }
                else if (scancode == SDL_SCANCODE_RIGHT)
                {
                    coreInput.key = CoreInputKeyRight;
                }
                
                // console buttons
                if (keycode == SDLK_RETURN || keycode == SDLK_p)
                {
                    coreInput.pause = true;
                }
                
#if HOT_KEYS
                // system
                if (event.key.keysym.mod & KMOD_CTRL)
                {
                    if (keycode == SDLK_d)
                    {
                        core_setDebug(runner.core, !core_getDebug(runner.core));
                        if (core_getDebug(runner.core))
                        {
                            overlay_message(runner.core, "DEBUG ON");
                        }
                        else
                        {
                            overlay_message(runner.core, "DEBUG OFF");
                        }
                    }
                    else if (keycode == SDLK_f)
                    {
                        if (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP)
                        {
                            SDL_SetWindowFullscreen(window, 0);
                        }
                        else
                        {
                            SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                        }
                        updateMouseMode();
                    }
                    else if (keycode == SDLK_r)
                    {
                        if (hasProgram())
                        {
                            runMainProgram();
                            overlay_message(runner.core, "RELOADED");
                        }
                    }
                    else if (keycode == SDLK_e)
                    {
                        rebootNX();
                    }
                    else if (keycode == SDLK_s)
                    {
                        int scale = (event.key.keysym.mod & KMOD_SHIFT) ? 1 : 3;
                        saveScreenshot(scale);
                    }
                }
                else if (keycode == SDLK_ESCAPE)
                {
                    if (settings.session.disabledev)
                    {
                        quit = true;
                    }
#if DEV_MENU
                    else if (hasProgram())
                    {
                        if (mainState != MainStateDevMenu)
                        {
                            showDevMenu();
                        }
                    }
#endif
                }
#endif
                break;
            }
                
            case SDL_TEXTINPUT: {
                char key = event.text.text[0];
                hasInput = true;
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
                hasInput = true;
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
                hasInput = true;
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
    
    switch (mainState)
    {
        case MainStateUndefined:
            break;
            
        case MainStateBootIntro:
            if (hasInput && !hasProgram())
            {
                // user hint
                overlay_message(runner.core, "DRAG .NX INTO WINDOW");
            }
            core_update(runner.core, &coreInput);
            if (machine_peek(runner.core, bootIntroStateAddress) == 2)
            {
                machine_poke(runner.core, bootIntroStateAddress, 3);
#ifdef __EMSCRIPTEN__
                emscripten_async_wget(mainProgramFilename, mainProgramFilename, onloaded, onerror);
#else
                runMainProgram();
#endif
            }
            break;
            
        case MainStateRunningProgram:
        case MainStateRunningTool:
            core_update(runner.core, &coreInput);
            if (hasInput && !coreInput.out_hasUsedInput)
            {
                // user hints for controls
                union IOAttributes attr = runner.core->machine->ioRegisters.attr;
                if (attr.touchEnabled && !attr.keyboardEnabled)
                {
                    overlay_message(runner.core, "TOUCH/MOUSE");
                }
                if (attr.keyboardEnabled && !attr.touchEnabled)
                {
                    overlay_message(runner.core, "KEYBOARD");
                }
                if (attr.gamepadsEnabled && !attr.keyboardEnabled)
                {
                    if (attr.gamepadsEnabled == 2)
                    {
                        if (messageNumber % 2 == 1)
                        {
                            overlay_message(runner.core, "P2 ]:ESDF [:TAB \\:Q");
                        }
                        else
                        {
                            overlay_message(runner.core, "P1 ]:ARROWS [:N \\:M");
                        }
                        messageNumber++;
                    }
                    else
                    {
                        overlay_message(runner.core, "]:ARROWS [:Z \\:X");
                    }
                }
            }
            break;
            
        case MainStateDevMenu:
#if DEV_MENU
            dev_update(&devMenu, &coreInput);
#endif
            break;
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
    
    video_renderScreen(runner.core, pixels);
    
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

void audioCallback(void *userdata, Uint8 *stream, int len)
{
    int16_t *samples = (int16_t *)stream;
    int numSamples = len / NUM_CHANNELS;
    audio_renderAudio(userdata, samples, numSamples, audioSpec.freq);
}

void saveScreenshot(int scale)
{
#if SCREENSHOTS
    void *pixels = NULL;
    int pitch = 0;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    bool succeeded = screenshot_save(pixels, scale);
    SDL_UnlockTexture(texture);
    if (succeeded)
    {
        overlay_message(runner.core, "SCREENSHOT SAVED");
    }
    else
    {
        overlay_message(runner.core, "SCREENSHOT ERROR");
    }
#endif
}

#ifdef __EMSCRIPTEN__

void onloaded(const char *filename)
{
    runMainProgram();
}

void onerror(const char *filename)
{
}

#endif
