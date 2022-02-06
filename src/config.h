
#ifndef CONFIG_H
#define CONFIG_H


#include "note.h"

#include <SDL2/SDL.h>

#include <string>


// constexpr double A4 defined in note.h!
constexpr Note LOWEST_NOTE = Note(Notes::E, 2);


/* Transcribe config */
constexpr int SAMPLE_RATE = 96000 * 2;
// constexpr int SAMPLE_RATE = 48000;
// constexpr int SAMPLE_RATE = 44100;

// High Res transcriber
constexpr int FRAME_SIZE = 1024 * 16 /** 2*/;  // Number of samples in Fourier frame
// constexpr int FRAME_SIZE = 1024 * 4 /** 2*/;  // Number of samples in Fourier frame
constexpr double POWER_THRESHOLD = 15.0;  // Threshold of channel power before finding peaks
constexpr double PEAK_THRESHOLD = 15.0;  // Threshold of peak before significant
constexpr double OVERTONE_ERROR = 10.0;  // Error in cents that an detected overtone may have compared to the theoretical overtone

// Overlapping read buffers
// Overlap is only supported if the same number of samples is requested every call to the SampleGetter
constexpr bool DO_OVERLAP = false;
// 0.0 < OVERLAP < 1.0: Ratio of old to new buffer, where higher numbers use more old buffer
constexpr double OVERLAP_RATIO = 0.1;
static_assert(OVERLAP_RATIO > 0.0 && OVERLAP_RATIO < 1.0, "Overlap ratio should be between 0.0 and 1.0");

// When reading from audio in, instead of reading a fixed ratio, read as many samples as possible without blocking
constexpr bool DO_OVERLAP_NONBLOCK = false;
// Minimum number of samples to overlap
// Should not be more than the size of a frame!
constexpr int MIN_NEW_SAMPLES_NONBLOCK = 14 * 1024;  // samples
constexpr int MAX_NEW_SAMPLES_NONBLOCK = (16 * 1024) - 1;  // samples
// constexpr int MIN_NEW_SAMPLES = 14;  // samples
// constexpr int MAX_NEW_SAMPLES = 90;  // samples
// constexpr int MIN_OVERLAP_ADVANCE = 1024 * 14;  // samples
static_assert(!(DO_OVERLAP && DO_OVERLAP_NONBLOCK), "Can't set both DO_OVERLAP and DO_OVERLAP_NONBLOCK");

// TODO: Remove, as is checked run-time by Program constructor (where the estimator is created)
constexpr int MAX_FRAME_SIZE = std::max(FRAME_SIZE, (int)round((double)SAMPLE_RATE / LOWEST_NOTE.freq));


/* Audio in/out config */
constexpr SDL_AudioFormat AUDIO_FORMAT = AUDIO_F32SYS;  // 32 bit floats
// constexpr SDL_AudioFormat AUDIO_FORMAT = AUDIO_S32SYS;  // 32 bit ints
constexpr unsigned int N_CHANNELS = 1;
constexpr unsigned int SAMPLES_PER_BUFFER = 512;

// Check if audio format can be converted to float32 (AUDIO_F32SYS)
static_assert(AUDIO_FORMAT != AUDIO_S32SYS || AUDIO_FORMAT != AUDIO_F32SYS, "Audio format has to be either int32 or float32");


constexpr bool DISPLAY_AUDIO_UNDERRUNS = false;


/* Graphics config */
constexpr bool HEADLESS = false;

// Maximum graphics frames per second (not to be confused with Fourier frames)
constexpr double MAX_FPS = 30.0;

constexpr bool DISPLAY_NOTE_LINES = false;

constexpr int DEFAULT_RES[2] = {1024, 768};
constexpr int MIN_RES[2] = {800, 600};

// Set to <=0 for full display
constexpr double DEFAULT_MAX_DISPLAY_FREQUENCY = 2000.0;
constexpr double MAX_FOURIER_FREQUENCY = (double)SAMPLE_RATE / 2.0;  // Hz
constexpr double MIN_FOURIER_FREQUENCY = 50.0;  // Hz

// Maximum number of previous data point to save in RAM for graphics (not used in headless mode)
constexpr int MAX_HISTORY_DATAPOINTS = 2000;

// Check if the maximum displayed frequency is lower than the maximum frequency from the Fourier transform
// static_assert(ceil(DEFAULT_MAX_DISPLAY_FREQUENCY / ((double)SAMPLE_RATE / (double)FRAME_SIZE)) < (FRAME_SIZE / 2) + 1, "DEFAULT_MAX_DISPLAY_FREQUENCY is too high; please set to <=0");
static_assert(DEFAULT_MAX_DISPLAY_FREQUENCY <= MAX_FOURIER_FREQUENCY, "DEFAULT_MAX_DISPLAY_FREQUENCY is too high; please set to <=0");


/* Results file config */
constexpr char DEFAULT_OUTPUT_FILENAME[] = "output.json";
constexpr int INDENT_AMOUNT = 4;  // Number of spaces per indent
constexpr bool WRITE_SILENCE = true;


// Easter egg constant time arpeggiator
// Don't forget to use a low frame size, as audio is played per frame
constexpr bool ENABLE_ARPEGGIATOR = false;
constexpr double NOTE_TIME = 100.0;  // ms


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
