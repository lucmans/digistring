#ifndef CONFIG_H
#define CONFIG_H


#include <SDL2/SDL.h>


const int SAMPLE_RATE = 96000 * 2;
const unsigned int SAMPLES_PER_BUFFER = 64;
const SDL_AudioFormat AUDIO_FORMAT = AUDIO_F32SYS;
const unsigned int N_CHANNELS = 1;

const int D_BUFFERS_PER_ACTION = 32;

const double MAX_FPS = 30.0;
const int FONT_SIZE = 30;
const SDL_Color TXT_COLOR = {0xff, 0xff, 0xff, 0xff};
const int RENDER_DOUBLE_PRECISION = 3;

const int RENDER_X_OFFSET = 100;
const int RENDER_Y_OFFSET = 100;


#endif  // CONFIG_H
