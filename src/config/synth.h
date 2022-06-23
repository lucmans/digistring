#ifndef DIGISTRING_CONFIG_SYNTH_H
#define DIGISTRING_CONFIG_SYNTH_H


// Print warnings about impossible to synthesize situations
constexpr bool PRINT_SYNTH_WARNINGS = true;

// Maximum difference in cent between frames before note is seen as a different note
constexpr double MAX_CENT_DIFFERENCE = 2.5;

// Frequency with which non-zero waves are zeroed with
constexpr double KILL_FREQ = 10000.0;

// Amount of synth volume change for every keypress
constexpr double D_SYNTH_VOLUME = 0.025;

// Number of samples in synth buffer when playing back note event files
constexpr int SYNTH_BUFFER_SIZE = 16384;


#endif  // DIGISTRING_CONFIG_SYNTH_H
