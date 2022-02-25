#ifndef DIGISTRING_CONFIG_CLI_ARGS_H
#define DIGISTRING_CONFIG_CLI_ARGS_H


#include "note.h"

#include <string>


// Struct with all settings that can be changed through CLI arguments
// Should only be written to by CLIArgParser at the start of the program
struct CLIArgs {
    int res_w = -1;
    int res_h = -1;
    bool fullscreen = false;

    // TODO: Clean (and absolute?) this string (like done on CLI passed rsc directory) for better error printing
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
extern CLIArgs cli_args;


#endif  // DIGISTRING_CONFIG_CLI_ARGS_H
