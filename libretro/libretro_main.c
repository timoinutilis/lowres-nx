//
// Copyright 2021 Timo Kloss, Antoine Fauroux
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

#include "libretro_main.h"
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "libretro.h"
#include "core.h"
#include "boot_intro.h"

#define SAMPLING_RATE 44100.0f
#define VIDEO_PIXELS SCREEN_WIDTH * SCREEN_HEIGHT
#define AUDIO_SAMPLES 1470

static struct retro_log_callback logging;
static retro_log_printf_t log;
static retro_environment_t environment_callback;
static retro_video_refresh_t video_refresh_callback;
static retro_audio_sample_t audio_sample_callback;
static retro_audio_sample_batch_t audio_sample_batch_callback;
static retro_input_poll_t input_poll_callback;
static retro_input_state_t input_state_callback;

static uint32_t *pixels;
static int16_t *audio_buf;

static struct Core *core = NULL;
static struct CoreDelegate coreDelegate;
static struct CoreInput coreInput;
static long ticks = 0;
static bool hasInput = false;
static bool hasUsedInputLastUpdate = false;
static bool messageShownUsingDisk = false;
static enum MainState mainState = MainStateUndefined;
static char *sourceCode = NULL;

void bootNX(void);
void runMainProgram(void);

void interpreterDidFail(void *context, struct CoreError coreError);
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager);
void diskDriveDidSave(void *context, struct DataManager *diskDataManager);
void diskDriveIsFull(void *context, struct DataManager *diskDataManager);
void controlsDidChange(void *context, struct ControlsInfo controlsInfo);
void persistentRamWillAccess(void *context, uint8_t *destination, int size);
void persistentRamDidChange(void *context, uint8_t *data, int size);


/* ======== LibRetro ======== */

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   va_list va;
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

void init_joysticks()
{
    struct retro_input_descriptor desc[] =
    {
        // Player 1
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
        {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
        
        // Player 2
        {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
        {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
        {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
        {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
        {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
        {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
        
        {0}
    };
    
    environment_callback(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

/**
 * Retrieve gamepad information from libretro.
 */
bool update_gamepad(int player)
{
    // D-Pad
    coreInput.gamepads[player].up = input_state_callback(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
    coreInput.gamepads[player].down = input_state_callback(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
    coreInput.gamepads[player].left = input_state_callback(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
    coreInput.gamepads[player].right = input_state_callback(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

    // A/B
    coreInput.gamepads[player].buttonA = input_state_callback(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
    coreInput.gamepads[player].buttonB = input_state_callback(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
    
    // Pause (Start button)
    //TODO: only on button press (first frame down)
    if (input_state_callback(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
    {
        coreInput.pause = true;
    }
    
    struct CoreInputGamepad gamepad = coreInput.gamepads[player];
    return gamepad.buttonA || gamepad.buttonB || gamepad.up || gamepad.down || gamepad.left || gamepad.right;
}

int mouse_pointer_convert(float coord, float full)
{
    float max = 0x7fff;
    return (int)((coord + max) / (max * 2.0f) * full);
}

bool update_mouse()
{
    // Get the Pointer X and Y, and convert it to screen position.
    coreInput.touchX = mouse_pointer_convert(input_state_callback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X), SCREEN_WIDTH);
    coreInput.touchY = mouse_pointer_convert(input_state_callback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y), SCREEN_HEIGHT);
    
    coreInput.touch = input_state_callback(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED);
    
    return coreInput.touch;
}

void keyboard_pressed(bool down, unsigned keycode, uint32_t character, uint16_t key_modifiers)
{
    if (down)
    {
        hasInput = true;
        
        if (character != 0)
        {
            // Text input
            if (character >= RETROK_SPACE &&  character <= RETROK_UNDERSCORE)
            {
                coreInput.key = character;
            }
            if (character >= RETROK_a &&  character <= RETROK_z)
            {
                coreInput.key = character - 32;
            }
        }
        
        switch (keycode)
        {
        case RETROK_RETURN:
            coreInput.key = CoreInputKeyReturn;
            break;
        case RETROK_BACKSPACE:
            coreInput.key = CoreInputKeyBackspace;
            break;
        case RETROK_UP:
            coreInput.key = CoreInputKeyUp;
            break;
        case RETROK_DOWN:
            coreInput.key = CoreInputKeyDown;
            break;
        case RETROK_LEFT:
            coreInput.key = CoreInputKeyLeft;
            break;
        case RETROK_RIGHT:
            coreInput.key = CoreInputKeyRight;
            break;
        }
    }
}

/* Sets callbacks. retro_set_environment() is guaranteed to be called
 * before retro_init().
 *
 * The rest of the set_* functions are guaranteed to have been called
 * before the first call to retro_run() is made. */

RETRO_API void retro_set_environment(retro_environment_t callback)
{
    environment_callback = callback;
    
    bool no_content = false;
    callback(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
    
    if (callback(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log = logging.log;
    else
        log = fallback_log;
    
    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    environment_callback(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);
    
    struct retro_keyboard_callback kcb = {
        keyboard_pressed
    };
    callback(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &kcb);
    
    //TODO: have a look at these:
    // RETRO_ENVIRONMENT_SET_MESSAGE
    // RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK
    // RETRO_ENVIRONMENT_SET_MEMORY_MAPS
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t callback)
{
    video_refresh_callback = callback;
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t callback)
{
    audio_sample_callback = callback;
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t callback)
{
    audio_sample_batch_callback = callback;
}

RETRO_API void retro_set_input_poll(retro_input_poll_t callback)
{
    input_poll_callback = callback;
}

RETRO_API void retro_set_input_state(retro_input_state_t callback)
{
    input_state_callback = callback;
}

/* Library global initialization/deinitialization. */

RETRO_API void retro_init(void)
{
    log(RETRO_LOG_INFO, "[LowRes NX] Initialization\n");
    
    core = calloc(1, sizeof(struct Core));
    if (core)
    {
        core_init(core);
        
        coreDelegate.interpreterDidFail = interpreterDidFail;
        coreDelegate.diskDriveWillAccess = diskDriveWillAccess;
        coreDelegate.diskDriveDidSave = diskDriveDidSave;
        coreDelegate.diskDriveIsFull = diskDriveIsFull;
        coreDelegate.controlsDidChange = controlsDidChange;
        coreDelegate.persistentRamWillAccess = persistentRamWillAccess;
        coreDelegate.persistentRamDidChange = persistentRamDidChange;
        
        core_setDelegate(core, &coreDelegate);
    }
    
    pixels = calloc(VIDEO_PIXELS, sizeof(uint32_t));
    audio_buf = calloc(AUDIO_SAMPLES, sizeof(int16_t));
    
    init_joysticks();
    
    bootNX();
}

RETRO_API void retro_deinit(void)
{
    log(RETRO_LOG_INFO, "[LowRes NX] Deinitialization\n");
    
    if (core)
    {
        core_deinit(core);
        free(core);
        core = NULL;
    }
    
    if (pixels)
    {
        free(pixels);
        pixels = NULL;
    }
    
    if (audio_buf)
    {
        free(audio_buf);
        audio_buf = NULL;
    }
}

/* Must return RETRO_API_VERSION. Used to validate ABI compatibility
 * when the API is revised. */
RETRO_API unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

/* Gets statically known system info. Pointers provided in *info
 * must be statically allocated.
 * Can be called at any time, even before retro_init(). */
RETRO_API void retro_get_system_info(struct retro_system_info *info)
{
    memset(info, 0, sizeof(*info));
    info->library_name     = "LowRes NX";
    info->library_version  = CORE_VERSION;
    info->need_fullpath    = false;
    info->valid_extensions = "nx";
}

/* Gets information about system audio/video timings and geometry.
 * Can be called only after retro_load_game() has successfully completed.
 * NOTE: The implementation of this function might not initialize every
 * variable if needed.
 * E.g. geom.aspect_ratio might not be initialized if core doesn't
 * desire a particular aspect ratio. */
RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info)
{
    info->timing.fps = 60.0;
    info->timing.sample_rate = SAMPLING_RATE;
    
    info->geometry.base_width = SCREEN_WIDTH;
    info->geometry.base_height = SCREEN_HEIGHT;
    info->geometry.max_width = SCREEN_WIDTH;
    info->geometry.max_height = SCREEN_HEIGHT;
    info->geometry.aspect_ratio = 0.0f;
    
}

/* Sets device to be used for player 'port'.
 * By default, RETRO_DEVICE_JOYPAD is assumed to be plugged into all
 * available ports.
 * Setting a particular device type is not a guarantee that libretro cores
 * will only poll input based on that particular device type. It is only a
 * hint to the libretro core when a core cannot automatically detect the
 * appropriate input device type on its own. It is also relevant when a
 * core can change its behavior depending on device type. */
RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
{
}

/* Resets the current game. */
RETRO_API void retro_reset(void)
{
    log(RETRO_LOG_INFO, "[LowRes NX] Reset\n");
    runMainProgram();
}

/* Runs the game for one video frame.
 * During retro_run(), input_poll callback must be called at least once.
 *
 * If a frame is not rendered for reasons where a game "dropped" a frame,
 * this still counts as a frame, and retro_run() should explicitly dupe
 * a frame if GET_CAN_DUPE returns true.
 * In this case, the video callback can take a NULL argument for data.
 */
RETRO_API void retro_run(void)
{
    input_poll_callback();
    
    if (core && pixels && audio_buf)
    {
        for (int i = 0; i < NUM_GAMEPADS; ++i)
        {
            if (update_gamepad(i))
            {
                hasInput = true;
            }
        }
        
        if (update_mouse())
        {
            hasInput = true;
        }
        
        switch (mainState)
        {
            case MainStateUndefined:
                break;
                
            case MainStateBootIntro:
                core_update(core, &coreInput);
                if (machine_peek(core, bootIntroStateAddress) == BootIntroStateReadyToRun)
                {
                    machine_poke(core, bootIntroStateAddress, BootIntroStateDone);
                    runMainProgram();
                }
                break;
                
            case MainStateRunningProgram:
                core_update(core, &coreInput);
                if (hasInput)
                {
                    if (core->interpreter->state == StateEnd)
                    {
                        overlay_message(core, "END OF PROGRAM");
                    }
                    else if (!coreInput.out_hasUsedInput && !hasUsedInputLastUpdate)
                    {
                        // user hints for controls
                        union IOAttributes attr = core->machine->ioRegisters.attr;
                        if (attr.touchEnabled && !attr.keyboardEnabled)
                        {
                            overlay_message(core, "TOUCH/MOUSE");
                        }
                        if (attr.keyboardEnabled && !attr.touchEnabled)
                        {
                            overlay_message(core, "KEYBOARD");
                        }
                        if (attr.gamepadsEnabled && !attr.keyboardEnabled)
                        {
                            overlay_message(core, "GAMEPAD");
                        }
                    }
                }
                break;
        }
        
        hasUsedInputLastUpdate = coreInput.out_hasUsedInput;
        
        video_renderScreen(core, pixels);
        video_refresh_callback(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, sizeof(uint32_t) * SCREEN_WIDTH);
        
        audio_renderAudio(core, audio_buf, AUDIO_SAMPLES, SAMPLING_RATE, 0);
        audio_sample_batch_callback(audio_buf, AUDIO_SAMPLES / 2);
    }
    
    hasInput = false;
    ++ticks;
}

/* Returns the amount of data the implementation requires to serialize
 * internal state (save states).
 * Between calls to retro_load_game() and retro_unload_game(), the
 * returned size is never allowed to be larger than a previous returned
 * value, to ensure that the frontend can allocate a save state buffer once.
 */
RETRO_API size_t retro_serialize_size(void)
{
    return 0;
}

/* Serializes internal state. If failed, or size is lower than
 * retro_serialize_size(), it should return false, true otherwise. */

RETRO_API bool retro_serialize(void *data, size_t size)
{
    return false;
}

RETRO_API bool retro_unserialize(const void *data, size_t size)
{
    return false;
}

RETRO_API void retro_cheat_reset(void)
{
}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
}

/* Loads a game. */
RETRO_API bool retro_load_game(const struct retro_game_info *game)
{
    log(RETRO_LOG_INFO, "[LowRes NX] Load game\n");
    
    if (core && game && game->data)
    {
        sourceCode = calloc(1, game->size + 1); // +1 for terminator
        if (sourceCode)
        {
            memcpy(sourceCode, game->data, game->size);
            if (mainState == MainStateBootIntro)
            {
                machine_poke(core, bootIntroStateAddress, BootIntroStateProgramAvailable);
            }
            else
            {
                runMainProgram();
            }
            return true;
        }
    }
    return false;
}

/* Loads a "special" kind of game. Should not be used,
 * except in extreme cases. */
RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
    return false;
}

/* Unloads a currently loaded game. */
RETRO_API void retro_unload_game(void)
{
    if (core)
    {
        core_willSuspendProgram(core);
    }
    
    if (sourceCode)
    {
        free(sourceCode);
        sourceCode = NULL;
    }
}

/* Gets region of game. */
RETRO_API unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

/* Gets region of memory. */
RETRO_API void *retro_get_memory_data(unsigned id)
{
    switch (id)
    {
        case RETRO_MEMORY_SAVE_RAM:
            return core->machine->persistentRam;
        default:
            return NULL;
    }
}

RETRO_API size_t retro_get_memory_size(unsigned id)
{
    switch (id)
    {
        case RETRO_MEMORY_SAVE_RAM:
            return PERSISTENT_RAM_SIZE;
        default:
            return 0;
    }
}

/* ======== LowRes NX ======== */

void bootNX()
{
    if (!core) return;
    
    mainState = MainStateBootIntro;
    
    struct CoreError error = core_compileProgram(core, bootIntroSourceCode);
    if (error.code != ErrorNone)
    {
        core_traceError(core, error);
    }
    
    core->interpreter->debug = false;
    core_willRunProgram(core, ticks / 60);
}

void runMainProgram()
{
    if (!core || !sourceCode) return;
    
    core_willSuspendProgram(core);
    
    struct CoreError error = core_compileProgram(core, sourceCode);
    if (error.code != ErrorNone)
    {
        core_traceError(core, error);
    }
    else
    {
        core_willRunProgram(core, ticks / 60);
        mainState = MainStateRunningProgram;
    }
    
    messageShownUsingDisk = false;
}

/** Called on error */
void interpreterDidFail(void *context, struct CoreError coreError)
{
    core_traceError(core, coreError);
}

/** Returns true if the disk is ready, false if not. In case of not, core_diskLoaded must be called when ready. */
bool diskDriveWillAccess(void *context, struct DataManager *diskDataManager)
{
    if (!messageShownUsingDisk)
    {
        overlay_message(core, "NO DISK");
        messageShownUsingDisk = true;
    }
    return true;
}

/** Called when a disk data entry was saved */
void diskDriveDidSave(void *context, struct DataManager *diskDataManager)
{
    overlay_message(core, "NO DISK");
}

/** Called when a disk data entry was tried to be saved, but the disk is full */
void diskDriveIsFull(void *context, struct DataManager *diskDataManager)
{
}

/** Called when keyboard or gamepad settings changed */
void controlsDidChange(void *context, struct ControlsInfo controlsInfo)
{
}

/** Called when persistent RAM will be accessed the first time */
void persistentRamWillAccess(void *context, uint8_t *destination, int size)
{
}

/** Called when persistent RAM should be saved */
void persistentRamDidChange(void *context, uint8_t *data, int size)
{
}
