#ifndef DIGISTRING_CONFIG_TRANSCRIPTION_H
#define DIGISTRING_CONFIG_TRANSCRIPTION_H


#include "note.h"

#include <string>
#include <algorithm>


// constexpr double A4 defined in note.h!
constexpr Note LOWEST_NOTE = Note(Notes::E, 2);
constexpr Note HIGHEST_NOTE = Note(Notes::E, 6);


/* High Res transcriber */
constexpr int FRAME_SIZE = 1024 * 16 /** 2*/;  // Number of samples in Fourier frame
// constexpr int FRAME_SIZE = 4096;  // For 48000 Hz
// constexpr int FRAME_SIZE = 3763;  // For 44100 Hz
constexpr double POWER_THRESHOLD = 15.0;  // Threshold of channel power before finding peaks
constexpr double PEAK_THRESHOLD = 15.0;  // Threshold of peak before significant
constexpr double OVERTONE_ERROR = 10.0;  // Error in cents that an detected overtone may have compared to the theoretical overtone

// TODO: Remove, as is checked run-time by Program constructor (where the estimator is created)
constexpr int MAX_FRAME_SIZE = FRAME_SIZE;
// constexpr int MAX_FRAME_SIZE = std::max(FRAME_SIZE, (int)round((double)SAMPLE_RATE / LOWEST_NOTE.freq));

// Dolph Chebyshev attanuation
constexpr double DEFAULT_ATTENUATION = 50.0;  // dB (can't be <45 dB)

// Gaussian average settings (for peak picking)
constexpr int KERNEL_WIDTH = 47;  // Choose odd value
constexpr int MID = KERNEL_WIDTH / 2;
constexpr double SIGMA = 1.2;  // Higher values of sigma make values close to kernel center weight more
// constexpr double ENVELOPE_MIN = 0.1;  // Minimum height of envelope at peaks
constexpr double ENVELOPE_MIN = 0.25;  // Minimum height of envelope at peaks

// Min difference in Y value between last valley to be a peak
constexpr double MIN_PEAK_DY = 1.0;

// Filter notes which are outside of LOWEST_NOTE and HIGHEST_NOTE
constexpr bool LOW_HIGH_FILTER = true;

// Transient filtering
constexpr bool TRANSIENT_FILTER = false;
constexpr double TRANSIENT_FILTER_POWER = 0.3;  // Extra signal power over previous frame before frame has a transient


/* Overlapping read buffers */
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


#endif  // DIGISTRING_CONFIG_TRANSCRIPTION_H
