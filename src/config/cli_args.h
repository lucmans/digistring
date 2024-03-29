#ifndef DIGISTRING_CONFIG_CLI_ARGS_H
#define DIGISTRING_CONFIG_CLI_ARGS_H


#include "note.h"
#include "error.h"
#include "synth/synth.h"  // Only for Synths enum
#include "sample_getter/sample_getter.h"  // Only for SampleGetters enum

#include "config/audio.h"
#include "config/graphics.h"

#include <string>


// Due to how audio input method selection is currently implemented in ArgParser, this can't be changed (can't set to audio_in using a flag)
const SampleGetters DEFAULT_AUDIO_INPUT_METHOD = SampleGetters::audio_in;


// Struct with all settings that can be changed through CLI arguments
// Should only be written to by CLIArgParser at the start of the program
struct CLIArgs {
    int res_w = -1;
    int res_h = -1;
    bool fullscreen = false;

    // This path is verified (and string is cleaned) at start of main()
    std::string rsc_dir = "rsc/";

    // Print performance measurements to CLI every frame
    bool output_performance = false;
    // File to write performance number to (which can be plotted with the performance plot tool)
    std::string perf_output_file = "";  // Empty filename means "don't generate performance file"

    // Play recorded audio or synthesize audio based on estimated note
    bool playback = false;
    bool synth = false;
    Synths synth_type = Synths::sine_amped;
    double volume = 0.7;
    // Playback both separated over stereo options
    bool stereo_split = false;  // If true, implies cli_args.playback is true
    bool stereo_split_playback_left;  // == stereo_split_synth_right

    // Sync Digistring components (graphics etc.) with audio output rate
    // Program constructor may set value back to false if syncing is not a valid choice!
    bool sync_with_audio = false;

    // Audio input settings
    SampleGetters audio_input_method = DEFAULT_AUDIO_INPUT_METHOD;
    double generate_sine_freq = 1000.0;  // Hz
    Note generate_note_note = Note(Notes::A, 4);
    std::string play_file_name;

    // Audio device settings
    // Empty string will force passing NULL to let SDL select the best choice
    std::string in_dev_name = "";
    std::string out_dev_name = "";

    // JSON output settings
    bool output_file = false;
    std::string output_filename;

    bool midi_out = false;

    // Experiment execute
    bool do_experiment = false;
    std::string experiment_string;

    // Slowdown settings
    // Alters the number of retrieved samples and estimated note events after estimation
    // This tricks synthesizers and audio syncing by letting them think NoteEvents are longer by a factor of slowdown_factor
    // Only useful during development and may have serious detriments for real-time usage and performing experiments; use with caution
    // To prevent unnecessarily large synth_buffers to be allocated at start-up, we reallocate a larger synth_buffer if needed runtime
    bool do_slowdown = false;
    double slowdown_factor;

    // Playing back a note event file through an arbitrary synth
    bool do_play_note_event_file = false;
    std::string note_event_file;
};
extern CLIArgs cli_args;


// Checks if combination of cli_args is valid
inline bool verify_cli_args() {
    if(cli_args.do_slowdown && cli_args.output_file) {
        error("Outputting results is prohibited during slowdown, as rounding errors may cause tied notes to separate");
        return false;
    }

    if(cli_args.do_slowdown && cli_args.playback) {
        error("Can't playback with slowdown");
        return false;
    }

    if(cli_args.do_slowdown && !cli_args.sync_with_audio && !cli_args.synth) {
        error("Slowdown mode does nothing without syncing or synthesizing audio");
        hint("Either disable slowdown or pass --sync/--synth flag");
        return false;
    }

    if(cli_args.playback && cli_args.synth && !cli_args.stereo_split) {
        error("Can't playback input audio and synthesize audio at the same time without stereo splitting");
        hint("To split playback and synthesis over stereo channels, pass 'left' or 'right' to playback");
        return false;
    }

    if(cli_args.stereo_split && !cli_args.synth) {
        error("Passing 'left' or 'right' to playback flag has no effect without synthesizing");
        return false;
    }

    return true;
}


#endif  // DIGISTRING_CONFIG_CLI_ARGS_H
