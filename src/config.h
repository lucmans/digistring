
#ifndef CONFIG_H
#define CONFIG_H


#include "note.h"

#include <SDL2/SDL.h>

#include <string>


constexpr const double A4 = 440.0;  // Hz

const Note LOWEST_NOTE = Note(Notes::E, 2);


/* Transcribe config */
const int SAMPLE_RATE = 96000 * 2;
// const int SAMPLE_RATE = 48000;
// const int SAMPLE_RATE = 44100;

// High Res transcriber
const int FRAME_SIZE = 1024 * 16 /** 2*/;  // Number of samples in Fourier frame
const double POWER_THRESHOLD = 15.0;  // Threshold of channel power before finding peaks
const double PEAK_THRESHOLD = 15.0;  // Threshold of peak before significant
const double OVERTONE_ERROR = 10.0;  // Error in cents that an detected overtone may have compared to the theoretical overtone


/* Audio in/out config */
const SDL_AudioFormat AUDIO_FORMAT = AUDIO_F32SYS;  // 32 bit floats
// const SDL_AudioFormat AUDIO_FORMAT = AUDIO_S32SYS;  // 32 bit ints
const unsigned int N_CHANNELS = 1;
const unsigned int SAMPLES_PER_BUFFER = 512;

// Check if audio format can be converted to float32 (AUDIO_F32SYS)
static_assert(AUDIO_FORMAT != AUDIO_S32SYS || AUDIO_FORMAT != AUDIO_F32SYS, "Audio format has to be either int32 or float32");


/* Graphics config */
const bool HEADLESS = false;

const double MAX_FPS = 30.0;

const bool DISPLAY_NOTE_LINES = false;

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


// Struct with all settings that can be changed through CLI arguments
struct Settings {
    int res_w = DEFAULT_RES[0];
    int res_h = DEFAULT_RES[1];
    bool fullscreen = false;

    std::string rsc_dir = "rsc/";

    // Print performance measurements every frame
    bool output_performance = false;

    // Play recorded audio
    bool playback = false;

    // Generate sine instead of using audio input
    bool generate_sine = false;
    double generate_sine_freq = 1000.0;  // Hz

    bool generate_note = false;
    Note generate_note_note = Note(Notes::A, 4);

    bool play_file = false;
    std::string play_file_name;

    bool output_file = false;
    std::string output_filename;
};
extern Settings settings;


// Functions relating to signals and gracefully quitting through interrupt
void signal_handler(const int signum);

bool poll_quit();
void set_quit();
void reset_quit();


#endif  // CONFIG_H
