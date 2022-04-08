#ifndef DIGISTRING_CONFIG_AUDIO_H
#define DIGISTRING_CONFIG_AUDIO_H


#include <SDL2/SDL.h>


// Sampling rate of input (and output if playback is enabled)
constexpr int SAMPLE_RATE = 192000;
// constexpr int SAMPLE_RATE = 48000;
// constexpr int SAMPLE_RATE = 44100;


// When processing takes longer than playing the samples back, underruns occur
constexpr bool PRINT_AUDIO_UNDERRUNS = false;


constexpr SDL_AudioFormat AUDIO_FORMAT = AUDIO_F32SYS;  // 32 bit floats
// constexpr SDL_AudioFormat AUDIO_FORMAT = AUDIO_S32SYS;  // 32 bit ints
// Check if audio format can be converted to float32 (AUDIO_F32SYS)
static_assert(AUDIO_FORMAT != AUDIO_S32SYS || AUDIO_FORMAT != AUDIO_F32SYS, "Audio format has to be either int32 or float32");

constexpr unsigned int N_CHANNELS = 1;  // Mono
constexpr unsigned int SAMPLES_PER_BUFFER = 512;

// When reading samples from audio in, Digistring sleeps the time it takes to have enough samples ready from the audio driver
// To prevent sleeping too long, which leads to more latency, we only sleep SLEEP_FACTOR times wait required time
// Setting a lower factor increases the length of the burst of high CPU usage before full frame is read
// We still let the CPU burst to minimize the latency between the driver having the buffer ready and reading it in Digistring
// Set to 0.0 to not sleep at all
constexpr double SLEEP_FACTOR = 0.85;


/* Easter egg constant time arpeggiator */
// Don't forget to use a low frame size, as audio is played per frame
constexpr bool ENABLE_ARPEGGIATOR = false;
constexpr double NOTE_TIME = 100.0;  // ms


#endif  // DIGISTRING_CONFIG_AUDIO_H
