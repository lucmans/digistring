
#ifndef CONFIG_CLI_ARGS
#define CONFIG_CLI_ARGS


#include "note.h"

#include <string>


// Struct with all settings that can be changed through CLI arguments
struct CLIArgs {
    int res_w = -1;
    int res_h = -1;
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
extern CLIArgs cli_args;


#endif  // CONFIG_CLI_ARGS
