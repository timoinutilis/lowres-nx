

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "config.h"
#if __LIBRETRO__
#include "libretro.h"
#include "libretro_lowres.h"
#include "core.h" //Lowres-NX API
#include "runner.h"
#include "dev_menu.h"
#include "settings.h"
#include "system_paths.h"
#include "utils.h"
#include "boot_intro.h"

// From Libretro API
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;
static bool use_audio_cb;

// From LowRes NX or mine
static bool initialized = false;


//other vars
const char *defaultDisk = "Disk.nx";
const int bootIntroStateAddress = 0xA000;
static uint8_t *pixels;
static uint16_t *audio_buf;
bool hasInput = false;
long ticks = 0;

struct Runner runner;
struct Settings settings;
#if DEV_MENU
struct DevMenu devMenu;
#endif
struct CoreInput coreInput;

enum MainState mainState = MainStateUndefined;
char mainProgramFilename[FILENAME_MAX] = "";

int numJoysticks = 0;

bool quit = false;
static bool releasedTouch = false;
bool forceRender = false;
bool hasUsedInputLastUpdate = false;
int volume = 0; // 0 = max, it's a bit shift


void main_init();
int main_deinit();
void bootNX();
void rebootNX();
bool hasProgram();
const char *getMainProgramFilename();
void selectProgram(const char *filename);
void runMainProgram();
long get_ticks();
void runToolProgram(const char *filename);
void showDevMenu();
bool usesMainProgramAsDisk();
void getDiskFilename(char *outputString);
void getRamFilename(char *outputString);
int update_gamepad(int player);
int mouse_pointer_convert(float coord, float full);
int update_mouse();
void keyboard_pressed(bool down, unsigned keycode, uint32_t character, uint16_t keymod);
int update_inputs();
void changeVolume(int delta);
void init_joysticks(int numjoysticks);

// ---------------------    CALLING LOWRES NX API ---------------------

void main_init()
{
    memset(&coreInput, 0, sizeof(struct CoreInput));
    settings_init(&settings, mainProgramFilename, 0, 0);
    runner_init(&runner);
    pixels = (uint8_t*)malloc(VIDEO_PIXELS * sizeof(uint32_t));
    audio_buf = (uint16_t*)malloc(AUDIO_SAMPLES * sizeof(uint16_t));

    if (runner_isOkay(&runner))
    {
#if DEV_MENU
        dev_init(&devMenu, &runner, &settings);
#endif
        bootNX();
        if (mainProgramFilename[0] != 0)
        {
            machine_poke(runner.core, bootIntroStateAddress, 1);
        }        
    }
}

void init_joysticks(int numjoysticks){
    numJoysticks = numjoysticks;
    
    if(numJoysticks==1){
        struct retro_input_descriptor desc[] =
            {
                // Player 1
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
                {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
            };

        environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
    }else if(numJoysticks==2)
    {
        struct retro_input_descriptor desc[]=
        {
            // Player 1
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
            {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},

            {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left"},
            {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up"},
            {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down"},
            {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right"},
            {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
            {1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
        };

        environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
    }
}
    
int main_deinit()
{
    runner_deinit(&runner);
    free(pixels);
    pixels = NULL;
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
    core_willRunProgram(runner.core, get_ticks());
}

void rebootNX()
{
    core_willSuspendProgram(runner.core);
    
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
    core_willSuspendProgram(runner.core);
    
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
        core_willRunProgram(runner.core, get_ticks());
        mainState = MainStateRunningProgram;
    }
}

long get_ticks(){
    return ticks; 
}

void runToolProgram(const char *filename)
{
    core_willSuspendProgram(runner.core);
    
    struct CoreError error = runner_loadProgram(&runner, filename);
    if (error.code == ErrorNone)
    {
        mainState = MainStateRunningTool;
        runner.core->interpreter->debug = false;
        core_willRunProgram(runner.core, get_ticks());
    }
    else
    {
        core_traceError(runner.core, error);
    }
}

void showDevMenu()
{
#if DEV_MENU
    core_willSuspendProgram(runner.core);
    
    bool reload = (mainState == MainStateRunningTool);
    mainState = MainStateDevMenu;
    log_cb(RETRO_LOG_INFO, "[LowRes NX] Show dev menu\n");
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

void getRamFilename(char *outputString)
{
    char *prefPath = ".";
    if (prefPath)
    {
        strncpy(outputString, prefPath, FILENAME_MAX - 1);
        
        char *separator = strrchr(mainProgramFilename, PATH_SEPARATOR_CHAR);
        if (separator)
        {
            separator++;
            strncat(outputString, separator, FILENAME_MAX - 1);
        }
        else
        {
            strncat(outputString, mainProgramFilename, FILENAME_MAX - 1);
        }
        
        char *postfix = strrchr(outputString, '.');
        if (postfix)
        {
            *postfix = 0;
        }
        strncat(outputString, ".dat", FILENAME_MAX - 1);
        
    } else {
        outputString[0] = 0;
    }
}

/**
 * Retrieve gamepad information from libretro.
 */
int update_gamepad(int player)
{
    // D-Pad
    coreInput.gamepads[player].up = input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP);
    coreInput.gamepads[player].down = input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN);
    coreInput.gamepads[player].left = input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT);
    coreInput.gamepads[player].right = input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT);

	// A/B
    coreInput.gamepads[player].buttonA = input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A);
    coreInput.gamepads[player].buttonB = input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B);
    
    // Pause on start
    if(input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START))
    {
        coreInput.pause = true;
    }
    // Restart the game on select
    if(input_state_cb(player, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT))
    {
        retro_reset();
    }

    return coreInput.gamepads[player].up
        +coreInput.gamepads[player].down
        +coreInput.gamepads[player].left
        +coreInput.gamepads[player].right
        +coreInput.gamepads[player].buttonA
        +coreInput.gamepads[player].buttonB;

}

/**
 * Converts a Pointer API coordinates to screen pixel position.
 */
int mouse_pointer_convert(float coord, float full)
{
	float max = 0x7fff;
	return (int)((coord + max) / (max * 2.0f) * full);
}

void setMouseEnabled(bool enabled){
    //nothing to do for libretro
}

/**
 * Retrieve pointer information from libretro.
 */
int update_mouse(){

    // Mouse pressed
    bool pressed = input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_PRESSED);
 
    if(pressed && !coreInput.touch)
    {
        coreInput.touch = true;

        // Get the Pointer X and Y, and convert it to screen position.
        int touchX = mouse_pointer_convert(
            input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_X),
            SCREEN_WIDTH);
        int touchY = mouse_pointer_convert(
            input_state_cb(0, RETRO_DEVICE_POINTER, 0, RETRO_DEVICE_ID_POINTER_Y),
            SCREEN_HEIGHT);

        if(touchX * touchY > 0)
        {
            coreInput.touchX = touchX;
            coreInput.touchY = touchY;
        }

        char message[16];
        sprintf(message, "TOUCH %d %d", coreInput.touchX, coreInput.touchY);
        overlay_message(runner.core, message);
    }else if(!pressed){
        coreInput.touch = false;
    }

    return coreInput.touch>0;
}

void keyboard_pressed(bool down, unsigned keycode, uint32_t character, uint16_t keymod)
{
    hasInput = true;
    if (down)
    {
        if(character != 0)
        {
            // Text input
            if ( character >= RETROK_SPACE &&  character <= RETROK_UNDERSCORE)
            {
                coreInput.key = character;
            }
            if ( character >= RETROK_a &&  character <= RETROK_z)
            {
                coreInput.key = character - 32;
            }
        }    

        switch(keycode)
        {
        case RETROK_RETURN:
            coreInput.key = CoreInputKeyReturn;
            coreInput.pause = true;
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
        case RETROK_ESCAPE:
            quit = true;
            core_willSuspendProgram(runner.core);
            break;
        }

#if HOT_KEYS
        uint16_t ctrlPressed = keymod & 0x02;
        if(ctrlPressed)
        {
            switch(keycode)
            {
            case RETROK_d:
#if DEV_MENU
            if (mainState != MainStateDevMenu)
            {
                showDevMenu();
            }
#else
            core_setDebug(runner.core, !core_getDebug(runner.core));
            if (core_getDebug(runner.core))
            {
                overlay_message(runner.core, "DEBUG ON");
            }
            else
            {
                overlay_message(runner.core, "DEBUG OFF");
            }
#endif
            break;
            case RETROK_f:
                // Not available in libretro ?
                break;
            case RETROK_r:
                retro_reset();
                break;
            case RETROK_e:
                rebootNX();
                break;
            case RETROK_z:
                forceRender = true;
                break;
            case RETROK_PLUS:
                changeVolume(-1);
                break;
            case RETROK_MINUS:
                changeVolume(+1);
                break;
            }
        }
#endif
    }
}


/**
*  to be executed each frame
* - poll events
* - update states, etc...
* returns 0 if should quit
**/
int update_inputs()
{
   ticks++;

   // Let libretro know that we need updated input states.
   input_poll_cb();

   // user hints for controls
   union IOAttributes attr = runner.core->machine->ioRegisters.attr;
   if (attr.touchEnabled)
   {
      // Mouse Input
      hasInput += update_mouse();
    }
    if (attr.gamepadsEnabled && settings.session.mapping == 0)
    {
        // Gamepad Input
        for (int i = 0; i < attr.gamepadsEnabled; i++){
            hasInput += update_gamepad(i);
        }
    }
   
    // Just to be sure we got a bool
    hasInput = hasInput > 0;

    switch (mainState)
    {
        case MainStateUndefined:
            break;
            
        case MainStateBootIntro:
            if (hasInput && !hasProgram())
            {
                // user hint
                overlay_message(runner.core, "NO .NX CARTRIDGE");
            }
            core_update(runner.core, &coreInput);
            if (machine_peek(runner.core, bootIntroStateAddress) == 2)
            {
                machine_poke(runner.core, bootIntroStateAddress, 3);
                runMainProgram();
            }
            break;
            
        case MainStateRunningProgram:
        case MainStateRunningTool:
            core_update(runner.core, &coreInput);

            if (hasInput && runner.core->interpreter->state == StateEnd)
            {
                overlay_message(runner.core, "END OF PROGRAM");
            }
            break;
            
        case MainStateDevMenu:
#if DEV_MENU
            dev_update(&devMenu, &coreInput);
#endif
            break;
        hasInput = false;
    }
    
    hasUsedInputLastUpdate = coreInput.out_hasUsedInput;

    return quit;
}

/**
* Draw pixels to screen
**/
void videoCallback()
{
    if (core_shouldRender(runner.core) || forceRender)
    {
        video_renderScreen(runner.core, (uint32_t*) pixels);
        video_cb(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, sizeof(uint32_t)*SCREEN_WIDTH);
        
    }
}

void audioCallback()
{
    if (core_shouldRender(runner.core) || forceRender)
    {
        audio_renderAudio(runner.core, (int16_t *) audio_buf, AUDIO_SAMPLES, SAMPLING_RATE, volume);
        audio_batch_cb((const int16_t *) audio_buf, AUDIO_SAMPLES/2);
    }
}
void changeVolume(int delta)
{
    volume += delta;
    if (volume < 0) volume = 0;
    if (volume > 6) volume = 6;
    char message[16];
    sprintf(message, "VOLUME %d%%", 100 >> volume);
    overlay_message(runner.core, message);
}

//#endif




//---------------------------   LIBRETRO API FUNCTION ---------------------------

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
   (void)level;
   va_list va;
   va_start(va, fmt);
   vfprintf(stderr, fmt, va);
   va_end(va);
}

void retro_init(void)
{
   if(!initialized)
   {
      log_cb(RETRO_LOG_INFO, "[LowRes NX] Initialization\n");
      main_init();
      initialized = true;
   }
}

void retro_deinit(void)
{
   if(initialized)
   {
      log_cb(RETRO_LOG_INFO, "[LowRes NX] Deinitialization\n");
      main_deinit();
      initialized = false;
   }
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

void retro_get_system_info(struct retro_system_info *info)
{
   memset(info, 0, sizeof(*info));
   info->library_name     = "LowRes NX";
   info->library_version  = "v"CORE_VERSION;
   info->need_fullpath    = true;
   info->valid_extensions = "nx";
}


void retro_get_system_av_info(struct retro_system_av_info *info)
{
    float aspect = SCREEN_WIDTH / SCREEN_HEIGHT;
    float sampling_rate = SAMPLING_RATE;

    info->timing = (struct retro_system_timing){
        .fps = 60.0,
        .sample_rate = sampling_rate,
    };

    info->geometry = (struct retro_game_geometry){
        .base_width = SCREEN_WIDTH,
        .base_height = SCREEN_HEIGHT,
        .max_width = SCREEN_WIDTH,
        .max_height = SCREEN_HEIGHT,
        .aspect_ratio = -1,
    };
}

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;

   bool no_content = true;
   cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

   if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
      log_cb = logging.log;
   else
      log_cb = fallback_log;

         environ_cb = cb;

   struct retro_keyboard_callback kcb = {
    keyboard_pressed
    };
   environ_cb(RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK, &kcb);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
   audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_reset(void)
{
    log_cb(RETRO_LOG_INFO, "[LowRes NX] Reseting game\n");
    if (hasProgram())
    {
        runMainProgram();
        overlay_message(runner.core, "RELOADED");
    }
}

static void audio_set_state(bool enable)
{
   (void)enable;
}

// Main function runned once per frame
void retro_run(void)
{
   //log_cb(RETRO_LOG_INFO, "[LowRes NX] Retro run\n");
   if (!initialized)return;
   
   bool quit = update_inputs();
   if(quit){
      retro_deinit();
      environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
      return;
   }

   videoCallback();
   audioCallback();
}

bool retro_load_game(const struct retro_game_info *info)
{
   log_cb(RETRO_LOG_INFO, "[LowRes NX] Loading ROM\n");
   if(!initialized)
      retro_init();

   // Pixel format.
   enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
	if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
		log_cb(RETRO_LOG_ERROR, "[LowRes NX] RETRO_PIXEL_FORMAT_XRGB8888 is not supported.\n");
		return false;
	}

    // Check for the content.
	if (info == NULL || info->path == NULL) {
		log_cb(RETRO_LOG_ERROR, "[LowRes NX] No game path provided.\n");
        bootNX();

        return true;
    }
   selectProgram(info->path);
   return true;
}

void retro_unload_game(void)
{
    if(initialized)
    {
        core_willSuspendProgram(runner.core);
        bootNX();
    }
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
   return false;
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *data_, size_t size)
{
    
    log_cb(RETRO_LOG_INFO, "[LowRes NX] Serializing NIY\n");
    /*if (!initialized && size < sizeof(struct Core))
      return false;

    data_ = (uint8_t *) runner.core->machine;
    */
    return false;
}

bool retro_unserialize(const void *data_, size_t size)
{
    log_cb(RETRO_LOG_INFO, "[LowRes NX] Unserializing NIY\n");
    /*if (!initialized && size < sizeof(struct Core))
      return false;
    runner.core = (struct Core *) data_;
    */
    return false;
}

void *retro_get_memory_data(unsigned id)
{
   (void)id;
   return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
   (void)id;
   return 0;
}

void retro_cheat_reset(void)
{}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
   (void)index;
   (void)enabled;
   (void)code;
}

#endif