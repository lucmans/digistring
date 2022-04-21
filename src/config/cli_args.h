#ifndef DIGISTRING_CONFIG_CLI_ARGS_H
#define DIGISTRING_CONFIG_CLI_ARGS_H


#include "note.h"
#include "synth/synth.h"  // Only for Synths enum
#include "sample_getter/sample_getter.h"  // Only for SampleGetters enum

#include <string>


// Due to how audio input method selection is currently implemented in ArgParser, this can't be changed
const SampleGetters DEFAULT_AUDIO_INPUT_METHOD = SampleGetters::audio_in;


// Struct with all settings that can be changed through CLI arguments
// Should only be written to by CLIArgParser at the start of the program
struct CLIArgs {
    int res_w = -1;
    int res_h = -1;
    bool fullscreen = false;

    // This path is verified (and string is cleaned) at start of main()
    std::string rsc_dir = "rsc/";

    // Print performance measurements every frame
    bool output_performance = false;

    // Play recorded audio or synthesize audio based on estimated note
    bool playback = false;
    bool synth = false;
    Synths synth_type = Synths::sine;

    // Audio input settings
    SampleGetters audio_input_method = DEFAULT_AUDIO_INPUT_METHOD;
    double generate_sine_freq = 1000.0;  // Hz
    Note generate_note_note = Note(Notes::A, 4);
    std::string play_file_name;

    // JSON output settings
    bool output_file = false;
    std::string output_filename;
};
extern CLIArgs cli_args;


#endif  // DIGISTRING_CONFIG_CLI_ARGS_H
