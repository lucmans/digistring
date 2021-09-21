
#ifndef CONFIG_H
#define CONFIG_H


#include <SDL2/SDL.h>


/* Audio config */
// Transcribe config
const int SAMPLE_RATE = 96000 * 2;
const int FRAME_SIZE = 1024 * 16 /** 2*/;  // Number of samples in Fourier frame

// Audio driver config
const SDL_AudioFormat FORMAT = AUDIO_F32SYS;
const unsigned int N_CHANNELS = 1;
const unsigned int SAMPLES_PER_BUFFER = 512;


/* Graphics config */
const int DEFAULT_RES[2] = {1024, 768};
const int MIN_RES[2] = {800, 600};


// Struct with all settings that can be changed through CLI arguments
struct Settings {
    int w = DEFAULT_RES[0];
    int h = DEFAULT_RES[1];
    bool fullscreen = false;

    // Run without window/graphics
    bool headless = false;

    // Print performance measurements every frame
    bool output_performance = false;

    // Play recorded audio
    bool playback = false;

    // Generate sine instead of using audio input
    bool generate_sine = false;
    double generate_sine_freq = 1000.0;  // Hz
};
extern Settings settings;


// Functions relating to signals and gracefully quitting through interrupt
void signal_handler(const int signum);

bool poll_quit();
void set_quit();
void reset_quit();


#endif  // CONFIG_H
