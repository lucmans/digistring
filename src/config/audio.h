
#ifndef CONFIG_AUDIO_H
#define CONFIG_AUDIO_H


#include <SDL2/SDL.h>


// Sampling rate of input (and output if playback is enabled)
constexpr int SAMPLE_RATE = 96000 * 2;
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


/* Easter egg constant time arpeggiator */
// Don't forget to use a low frame size, as audio is played per frame
constexpr bool ENABLE_ARPEGGIATOR = false;
constexpr double NOTE_TIME = 100.0;  // ms


#endif  // CONFIG_AUDIO_H
