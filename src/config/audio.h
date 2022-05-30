#ifndef DIGISTRING_CONFIG_AUDIO_H
#define DIGISTRING_CONFIG_AUDIO_H


#include <SDL2/SDL.h>


// Sampling rate of input (and output if playback is enabled)
constexpr int SAMPLE_RATE = 192000;
// constexpr int SAMPLE_RATE = 48000;
// constexpr int SAMPLE_RATE = 44100;

constexpr SDL_AudioFormat AUDIO_FORMAT = AUDIO_F32SYS;  // 32 bit floats
// constexpr SDL_AudioFormat AUDIO_FORMAT = AUDIO_S32SYS;  // 32 bit ints
// Check if audio format can be converted to float32 (AUDIO_F32SYS)
static_assert(AUDIO_FORMAT != AUDIO_S32SYS || AUDIO_FORMAT != AUDIO_F32SYS, "Audio format has to be either int32 or float32");

constexpr unsigned int SAMPLES_PER_BUFFER = 64;


// If the audio hardware doesn't support the requested audio settings, SDL can implicitly convert the given audio data
// With this setting, we can disallow this implicit conversion and generate error instead
constexpr bool ALLOW_PLAYBACK_CHANGE = true;


// When processing takes longer than playing the samples back, underruns occur
constexpr bool PRINT_AUDIO_UNDERRUNS = false;


// Alters the number of retrieved samples and estimated note events after estimation
// This tricks synthesizers and audio syncing by letting them think NoteEvents are longer by a factor of SLOWDOWN_FACTOR
// Only useful during development and may have serious detriments for real-time usage and performing experiments; use with caution
// To prevent unnecessarily large synth_buffers to be allocated at start-up, we reallocate a larger synth_buffer if needed runtime
constexpr bool SLOWDOWN = false;
constexpr double SLOWDOWN_FACTOR = 8.0;
static_assert(SLOWDOWN_FACTOR > 1.0, "SLOWDOWN_FACTOR must be > 1.0");


/* Note: Currently disabled! */
// When reading samples from audio in, Digistring sleeps the time it takes to have enough samples ready from the audio driver
// To prevent sleeping too long, which leads to more latency, we only sleep SLEEP_FACTOR times wait required time
// Setting a lower factor increases the length of the burst of high CPU usage before full frame is read
// We still let the CPU burst to minimize the latency between the driver having the buffer ready and reading it in Digistring
// Set to 0.0 to not sleep at all
constexpr double SLEEP_FACTOR = 0.85;
// Alternatively, subtract a constant time from sleep time to account for OS scheduling
constexpr double SLEEP_OVERHEAD_TIME = 15.0;  // Milliseconds


// Number of seconds is scrubbed through the input file every scroll wheel action
constexpr double SECONDS_PER_SCROLL = 0.1;


/* Easter egg constant time arpeggiator */
// Don't forget to use a low frame size, as audio is played per frame
constexpr bool ENABLE_ARPEGGIATOR = false;
constexpr double NOTE_TIME = 100.0;  // ms


#endif  // DIGISTRING_CONFIG_AUDIO_H
