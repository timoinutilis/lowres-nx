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

#include <stdbool.h>
#if __LIBRETRO__

#include "libretro.h"
#define SAMPLING_RATE 44100.0f
#define VIDEO_PIXELS SCREEN_WIDTH * SCREEN_HEIGHT
#define AUDIO_SAMPLES 1470

enum MainState {
    MainStateUndefined,
    MainStateBootIntro,
    MainStateRunningProgram,
    MainStateRunningTool,
    MainStateDevMenu,
};

enum Zoom {
    ZoomPixelPerfect,
    ZoomLarge,
    ZoomOverscan,
    ZoomSqueeze,
};

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
void setMouseEnabled(bool enabled);
void changeVolume(int delta);
void init_joysticks(int numjoysticks);


#endif
