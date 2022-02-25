
#ifndef CONFIG_GRAPHICS_H
#define CONFIG_GRAPHICS_H


#include "config/audio.h"


// For disabling graphics
constexpr bool HEADLESS = false;

// Maximum graphics frames per second (not to be confused with Fourier frames)
constexpr double MAX_FPS = 30.0;

// Show a vertical line at note frequencies in spectrum
constexpr bool DISPLAY_NOTE_LINES = false;

// Render the number of input samples queued, which shows audio overruns
constexpr bool DISPLAY_QUEUED_IN_SAMPLES = true;

// Draw a highlighted dot in spectrogram denoting a measured point
constexpr bool DRAW_MEASURED_POINTS = false;

// Draw the envelope used for peak detection in the high res estimator
constexpr bool RENDER_ENVELOPE = true;

// Draw the peaks found by the high res estimator
constexpr bool RENDER_PEAKS = true;

constexpr int DEFAULT_RES[2] = {1024, 768};
constexpr int MIN_RES[2] = {800, 600};

// Set to <=0 for full display
constexpr double DEFAULT_MAX_DISPLAY_FREQUENCY = 2000.0;
constexpr double MAX_FOURIER_FREQUENCY = (double)SAMPLE_RATE / 2.0;  // Hz
constexpr double MIN_FOURIER_FREQUENCY = 50.0;  // Hz

// Maximum number of previous data point to save in RAM for graphics (not used in headless mode)
constexpr int MAX_HISTORY_DATAPOINTS = 2000;

// Maximum width of a waterfall line texture (when only smaller textures are supported on specific system)
constexpr unsigned int MAX_PIXELS_WATERFALL_LINE = 16384;

// Determines if the waterfall flows up or down
constexpr bool WATERFALL_FLOW_DOWN = true;

// Check if the maximum displayed frequency is lower than the maximum frequency from the Fourier transform
// static_assert(ceil(DEFAULT_MAX_DISPLAY_FREQUENCY / ((double)SAMPLE_RATE / (double)FRAME_SIZE)) < (FRAME_SIZE / 2) + 1, "DEFAULT_MAX_DISPLAY_FREQUENCY is too high; please set to <=0");
static_assert(DEFAULT_MAX_DISPLAY_FREQUENCY <= MAX_FOURIER_FREQUENCY, "DEFAULT_MAX_DISPLAY_FREQUENCY is too high; please set to <=0");


#endif  // CONFIG_GRAPHICS_H
