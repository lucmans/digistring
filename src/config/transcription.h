#ifndef DIGISTRING_CONFIG_TRANSCRIPTION_H
#define DIGISTRING_CONFIG_TRANSCRIPTION_H


#include "note.h"

#include <string>
#include <algorithm>


// constexpr double A4 defined in note.h!
constexpr Note LOWEST_NOTE = Note(Notes::E, 2);
constexpr Note HIGHEST_NOTE = Note(Notes::E, 6);


/* High Res transcriber */
// constexpr int FRAME_SIZE = 40960;  // BasicFourier
constexpr int FRAME_SIZE = 1024 * 8 /** 2*/;  // Number of samples in Fourier frame
// constexpr int FRAME_SIZE = 1024 * 4;  // For 96000 Hz
// constexpr int FRAME_SIZE = 2048;  // For 48000 Hz
// constexpr int FRAME_SIZE = 1882;  // For 44100 Hz
constexpr double POWER_THRESHOLD = 15.0;  // Threshold of channel power before finding peaks
constexpr double PEAK_THRESHOLD = 0.25;  // Threshold of peak before significant
constexpr double OVERTONE_ERROR = 45.0;  // Error in cents that an detected overtone may have compared to the theoretical overtone

constexpr double ZERO_PAD_FACTOR = 15.0;  // Calculates number of zeros to pad; choose a power of two minus one for optimal efficiency (0.0 to disable)
constexpr int FRAME_SIZE_PADDED = FRAME_SIZE + (FRAME_SIZE * ZERO_PAD_FACTOR);  // Size of the frame with padding

// Dolph Chebyshev attanuation
constexpr double DEFAULT_ATTENUATION = 50.0;  // dB (shouldn't be <45 dB, as equivalent noise bandwidth will increase much)

// Gaussian average settings (for peak picking)
// constexpr double KERNEL_WIDTH_FACTOR = 0.000478;  // Width of kernel with respect to spectrum size
constexpr double KERNEL_WIDTH_FACTOR = 0.0005;  // Width of kernel with respect to spectrum size
// constexpr double KERNEL_WIDTH_FACTOR = 0.001;  // Width of kernel with respect to spectrum size
// constexpr double KERNEL_WIDTH_FACTOR = 0.002;  // Width of kernel with respect to spectrum size
constexpr int KERNEL_WIDTH = (FRAME_SIZE_PADDED * KERNEL_WIDTH_FACTOR) + ((int)(FRAME_SIZE_PADDED * KERNEL_WIDTH_FACTOR) % 2 == 0 ? 1 : 0);
// constexpr int KERNEL_WIDTH = 47;  // Choose odd value
constexpr double SIGMA = 1.25;  // Higher values of sigma make values close to kernel center weight more
/* TODO: Change to ENVELOPE_THRESHOLD */
// constexpr double ENVELOPE_MIN = 0.1;  // Minimum height of envelope at peaks
constexpr double ENVELOPE_MIN = 0.35;  // Minimum height of envelope at peaks

// Min difference in Y value between last valley to be a peak
constexpr double MIN_PEAK_DY = 1.0;

// Filter notes which are outside of LOWEST_NOTE and HIGHEST_NOTE
constexpr bool RANGE_FILTER = true;

// Transient filtering
constexpr bool TRANSIENT_FILTER = false;
constexpr double TRANSIENT_FILTER_POWER = 0.3;  // Extra signal power over previous frame before frame has a transient

/* TODO: Rename to SIGNAL_TO_NOISE_FACTOR */
constexpr double SIGNAL_TO_NOISE_FILTER = 0.05;  // Minimum height of peak compared to highest peak


/* Overlapping read buffers */
// Overlap is only supported if the same number of samples is requested every call to the SampleGetter
constexpr bool DO_OVERLAP = true;
// 0.0 < OVERLAP < 1.0: Ratio of old to new buffer, where higher numbers use more old buffer
constexpr double OVERLAP_RATIO = 0.85;
static_assert(OVERLAP_RATIO >= 0.0 && OVERLAP_RATIO <= 1.0, "Overlap ratio should be between 0.0 and 1.0");

// When reading from audio in, instead of reading a fixed ratio, read as many samples as possible without blocking
constexpr bool DO_OVERLAP_NONBLOCK = false;
constexpr double MIN_NONBLOCK_OVERLAP_RATIO = 0.65;
constexpr double MAX_NONBLOCK_OVERLAP_RATIO = 0.99999;
static_assert(MIN_NONBLOCK_OVERLAP_RATIO > 0.0 && MIN_NONBLOCK_OVERLAP_RATIO < 1.0, "Minimum non-blocking overlap ratio should be between 0.0 and 1.0");
static_assert(MAX_NONBLOCK_OVERLAP_RATIO > 0.0 && MAX_NONBLOCK_OVERLAP_RATIO < 1.0, "Maximum non-blocking overlap ratio should be between 0.0 and 1.0");
static_assert(MIN_NONBLOCK_OVERLAP_RATIO < MAX_NONBLOCK_OVERLAP_RATIO, "MAX_NONBLOCK_OVERLAP_RATIO can't be smaller than MIN_NONBLOCK_OVERLAP_RATIO");

static_assert(!(DO_OVERLAP && DO_OVERLAP_NONBLOCK), "Can't set both DO_OVERLAP and DO_OVERLAP_NONBLOCK");


#endif  // DIGISTRING_CONFIG_TRANSCRIPTION_H
