
#ifndef CONFIG_H
#define CONFIG_H


#include <SDL2/SDL.h>


/* Audio config */
// Transcribe config
const int SAMPLE_RATE = 96000 * 2;
const int FRAME_SIZE = 1024 * 16 /** 2*/;  // Number of samples in Fourier frame

// Audio driver config
const SDL_AudioFormat AUDIO_FORMAT = AUDIO_F32SYS;  // 32 bit floats
// const SDL_AudioFormat AUDIO_FORMAT = AUDIO_S32SYS;  // 32 bit ints
const unsigned int N_CHANNELS = 1;
const unsigned int SAMPLES_PER_BUFFER = 512;

// Check if audio format can be converted to float32 (AUDIO_F32SYS)
static_assert(AUDIO_FORMAT != AUDIO_S32SYS || AUDIO_FORMAT != AUDIO_F32SYS, "Audio format has to be either int32 or float32");


/* Graphics config */
const bool HEADLESS = false;

const int DEFAULT_RES[2] = {1024, 768};
const int MIN_RES[2] = {800, 600};

// Set to <=0 for full display
constexpr const double DEFAULT_MAX_DISPLAY_FREQUENCY = 2000.0;
// constexpr const double DEFAULT_MAX_DISPLAY_FREQUENCY = -1.0;  // DEBUG
constexpr const double MAX_FOURIER_FREQUENCY = (double)SAMPLE_RATE / 2.0;  // Hz
constexpr const double MIN_FOURIER_FREQUENCY = 50.0;  // Hz

// Maximum number of previous data point to save in RAM for graphics (not used in headless mode)
const int MAX_HISTORY_DATAPOINTS = 2000;

// Check if the maximum displayed frequency is lower than the maximum frequency from the Fourier transform
// static_assert(ceil(DEFAULT_MAX_DISPLAY_FREQUENCY / ((double)SAMPLE_RATE / (double)FRAME_SIZE)) < (FRAME_SIZE / 2) + 1, "DEFAULT_MAX_DISPLAY_FREQUENCY is too high; please set to <=0");
static_assert(DEFAULT_MAX_DISPLAY_FREQUENCY <= MAX_FOURIER_FREQUENCY, "DEFAULT_MAX_DISPLAY_FREQUENCY is too high; please set to <=0");


// namespace UI {
//     namespace max_display_frequency {
//         const int padding =
//     }
// }


// struct UI {

// }


// Struct with all settings that can be changed through CLI arguments
struct Settings {
    int w = DEFAULT_RES[0];
    int h = DEFAULT_RES[1];
    bool fullscreen = false;

    // // Run without window/graphics
    // bool headless = false;

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
