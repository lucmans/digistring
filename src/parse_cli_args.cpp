#include "parse_cli_args.h"
#include "generate_completions.cpp"

#include "note.h"
#include "init_sdl_audio.h"
#include "error.h"

#include "synth/synth.h"  // Note not synths.h

#include "config/cli_args.h"
#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"
#include "config/results_file.h"

#include <SDL2/SDL.h>  // Init SDL for checking audio devices

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <string>  // std::stoi(), std::stod()
#include <filesystem>  // basic checks on given rsc directory
#include <functional>  // std::invoke()
#include <optional>  // Construct note without initializing it
#include <map>
#include <stdexcept>  // std::runtime_error


// Initialize the global holding the chosen CLI argument options
CLIArgs cli_args;


bool flag_ordering(const std::string lhs, const std::string rhs) {
    // Normal sorting if both are - or both are --
    if((lhs[1] == '-' && rhs[1] == '-') || (lhs[1] != '-' && rhs[1] != '-'))
        return lhs < rhs;

    // If lhs is - and rhs is --, lhs should come before rhs
    if(lhs[1] != '-' && rhs[1] == '-')
        return true;

    // If lhs is -- and rhs is -, lhs should come after rhs
    if(lhs[1] == '-' && rhs[1] != '-')
        return false;

    error("Corner case reached in CLI flag sorting");
    exit(EXIT_FAILURE);
}

// const std::map<const std::string, const ParseObj> ArgParser::flag_to_func = {
const std::map<const std::string, const ParseObj, compare_func_t> ArgParser::flag_to_func = {
    {
        {"--audio",                 ParseObj(&ArgParser::parse_audio,                 {OptType::last_arg})},
        {"--audio_in",              ParseObj(&ArgParser::parse_audio_in,              {OptType::audio_in_device})},
        {"--audio_out",             ParseObj(&ArgParser::parse_audio_out,             {OptType::audio_out_device})},
        {"--experiment",            ParseObj(&ArgParser::parse_experiment,            {OptType::experiment})},
        {"--experiments",           ParseObj(&ArgParser::parse_experiments,           {OptType::last_arg})},
        {"-f",                      ParseObj(&ArgParser::parse_fullscreen,            {})},
        {"--file",                  ParseObj(&ArgParser::parse_file,                  {OptType::file})},
        {"--gen-completions",       ParseObj(&ArgParser::generate_completions,        {OptType::completions_file, OptType::last_arg})},
        {"-h",                      ParseObj(&ArgParser::parse_help,                  {OptType::last_arg})},
        {"--help",                  ParseObj(&ArgParser::parse_help,                  {OptType::last_arg})},
        {"--midi",                  ParseObj(&ArgParser::parse_midi_out,              {})},
        {"-n",                      ParseObj(&ArgParser::parse_generate_note,         {OptType::opt_note})},
        {"-o",                      ParseObj(&ArgParser::parse_output_file,           {OptType::output_file})},
        {"--output",                ParseObj(&ArgParser::parse_output_file,           {OptType::output_file})},
        {"--over",                  ParseObj(&ArgParser::parse_print_overtone,        {OptType::note, OptType::opt_integer, OptType::midi_switch, OptType::last_arg})},
        {"-p",                      ParseObj(&ArgParser::parse_playback,              {OptType::opt_left_right})},
        {"--play_note_event_file",  ParseObj(&ArgParser::parse_play_note_event_file,  {OptType::file, OptType::synth, OptType::opt_audio_out_device, OptType::last_arg})},
        {"--perf",                  ParseObj(&ArgParser::parse_print_performance,     {OptType::perf_file})},
        {"-r",                      ParseObj(&ArgParser::parse_resolution,            {OptType::integer, OptType::integer})},
        // {"--real-time",             ParseObj(&ArgParser::parse_sync_with_audio,     {})},
        {"--rsc",                   ParseObj(&ArgParser::parse_rsc_dir,               {OptType::dir})},
        {"-s",                      ParseObj(&ArgParser::parse_generate_sine,         {OptType::opt_decimal})},
        {"--slow",                  ParseObj(&ArgParser::parse_slow,                  {OptType::decimal})},
        {"--sync",                  ParseObj(&ArgParser::parse_sync_with_audio,       {})},
        {"--synth",                 ParseObj(&ArgParser::parse_synth,                 {OptType::opt_synth, OptType::opt_decimal})},
        {"--synths",                ParseObj(&ArgParser::parse_synths,                {OptType::last_arg})},
    },
    flag_ordering
};


const std::pair<const std::string, const std::string> help_strings[] = {
    {"--audio",                     "Print used audio driver and available audio devices"},
    {"--audio_in <device name>",    "Set the recording device to device name (as provided by Digistring at start-up"},
    {"--audio_out <device name>",   "Set the playback device to device name (as provided by Digistring at start-up"},
    {"--experiment <experiment>",   "Runs given experiment"},
    {"--experiments",               "Lists available experiments"},
    {"-f",                          "Start in fullscreen (also set the fullscreen resolution with '-r')"},
    {"--file <file>",               "Play samples from given file"},
    {"--gen-completions <file>",    "Generate Bash completions to file (overwriting it) (default filename is completions.sh)"},
    {"-h | --help",                 "Print command line argument information. Optionally pass 'readme' for readme formatting"},
    {"--midi",                      "Output MIDI events"},
    {"-n [note]",                   "Generate note (default is A4)"},
    {"-o | --output [file]",        "Write estimation results as JSON to file (default filename is " + DEFAULT_OUTPUT_FILENAME + ")"},
    {"--over <note> [n] [midi]",    "Print n (default is 5) overtones of given note; optionally toggle midi number column by passing \"midi_on\" or \"midi_off\" (default to midi_off)"},
    {"-p [left/right]",             "Play recorded audio back; when also synthesizing, pass \"left\" or \"right\" to set playback to this channel (and synthesis to the other)"},
    {"--perf <file>",               "Write performance statistics to file, which can be used by our `performance_plot` tool (may generate different files for different subtasks)"},
    {"-r <w> <h>",                  "Start GUI with given resolution"},
    // {"--real-time",                 "Run Digistring \"real-time\"; in other words, sync graphics etc. as if audio was playing back"},
    {"--rsc <path>",                "Set alternative resource directory location to path"},
    {"-s [f]",                      "Generate sine wave with frequency f (default is 1000.0 Hz) instead of using recording device"},
    {"--slow <factor>",             "Slowdown pitch estimation by the given factor"},
    {"--sync",                      "Run Digistring \"real-time\"; in other words, sync graphics etc. as if audio was playing back"},
    {"--synth [synth] [volume]",    "Synthesize sound based on note estimation from audio input (default synth is sine, default volume is 1.0)"},
    {"--synths",                    "List available synthesizers"},
};


void print_help() {
    std::cout << "Flags (argument parameters in <> are required and in [] are optional):" << std::endl;

    // Get largest flag width for formatting
    int max_flag_width = 0;
    for(const auto &[flag, desc] : help_strings) {
        const int flag_width = flag.size();
        if(flag_width > max_flag_width)
            max_flag_width = flag_width;
    }

    for(const auto &[flag, desc] : help_strings)
        std::cout << "  " << flag << std::string(max_flag_width - flag.size(), ' ') << " - " << desc << std::endl;
}


void print_help_readme_formatted() {
    std::cout << "Argument parameters in <> are required and in [] are optional.";
    for(const auto &[flag, desc] : help_strings)
        std::cout << "  \n`" << flag << "`: " << desc << ".";
    std::cout << std::endl;
}


void parse_args(const int argc, const char *const argv[]) {
    ArgParser arg_parser(argc, argv);
    arg_parser.parse_args();
}


ArgParser::ArgParser(const int _argc, const char *const _argv[]) : argc(_argc), argv(_argv) {
    cur_arg = 1;
}

ArgParser::~ArgParser() {

}


bool ArgParser::fetch_arg(const char *&arg) {
    arg = argv[cur_arg];

    // Do not check for (argv[cur_arg] == '\0'), as any arg may be '\0' by passing '' in the CLI
    if(cur_arg == argc)
        return false;

    cur_arg++;
    return true;
}

bool ArgParser::fetch_opt(const char *&arg) {
    arg = argv[cur_arg];

    // Do not check for (argv[cur_arg] == '\0'), as any arg may be '\0' by passing '' in the CLI
    if(cur_arg == argc)
        return false;

    if(arg[0] == '-')
        return false;

    cur_arg++;
    return true;
}


void ArgParser::parse_args() {
    const char *arg;
    while(fetch_arg(arg)) {
        try {
            const std::function<void(ArgParser&)> parse_func = ArgParser::flag_to_func.at(arg).function;
            parse_func(*this);
        }
        catch(const std::out_of_range &e) {
            error("Incorrect usage; flag '" + std::string(arg) + "' not known");
            __msg("");  // Prints newline
            print_help();
            exit(EXIT_FAILURE);
        }
        catch(const std::exception &e) {
            error("Failed to call function associated with given flag (" + STR(e.what()) + ")");
            exit(EXIT_FAILURE);
        }
    }
}



void ArgParser::parse_audio() {
    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        error("SDL could not initialize\nSDL Error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    print_audio_driver();
    __msg("");  // Print newline for clarity

    print_playback_devices();
    print_recording_devices();

    SDL_Quit();

    exit(EXIT_SUCCESS);
}


void ArgParser::parse_audio_in() {
    const char *in_dev_arg;
    if(!fetch_opt(in_dev_arg)) {
        error("No recording device name given");
        exit(EXIT_FAILURE);
    }

    cli_args.in_dev_name = in_dev_arg;
}

void ArgParser::parse_audio_out() {
    const char *out_dev_arg;
    if(!fetch_opt(out_dev_arg)) {
        error("No playback device name given");
        exit(EXIT_FAILURE);
    }

    cli_args.out_dev_name = out_dev_arg;
}


void ArgParser::parse_experiment() {
    const char *exp_cstr;
    if(!fetch_opt(exp_cstr)) {
        error("No experiment selected");
        exit(EXIT_FAILURE);
    }

    cli_args.do_experiment = true;
    cli_args.experiment_string = exp_cstr;
}


void ArgParser::parse_experiments() {
    auto it = str_to_experiment.cbegin();
    std::string experiments = it->first;
    for(++it; it != str_to_experiment.cend(); ++it)
        experiments += ", " + it->first;

    std::cout << "Available experiments: " << experiments << std::endl;
    exit(EXIT_SUCCESS);
}


void ArgParser::parse_fullscreen() {
    cli_args.fullscreen = true;
}


void ArgParser::parse_file() {
    if(cli_args.audio_input_method != DEFAULT_AUDIO_INPUT_METHOD) {
        std::string sample_getter_string;
        try {
            sample_getter_string = SampleGetterString.at(cli_args.audio_input_method);
        }
        catch(const std::out_of_range &e) {
            error("SampleGetter missing in SampleGetterString (in sample_getter/sample_getter.h)");
            exit(EXIT_FAILURE);
        }
        error("Can't play a file while using '" + sample_getter_string + "' as input method");
        exit(EXIT_FAILURE);
    }

    cli_args.audio_input_method = SampleGetters::audio_file;

    const char *arg;
    if(!fetch_opt(arg)) {
        error("No file given with the '--file' flag");
        exit(EXIT_FAILURE);
    }

    if(!std::filesystem::exists(arg)) {
        error("Given file does not exist");
        // debug("'" + STR(arg) + "'");
        exit(EXIT_FAILURE);
    }

    cli_args.play_file_name = arg;
}


void ArgParser::parse_help() {
    const char *arg;
    if(fetch_opt(arg)) {
        if(std::string(arg) == "readme") {
            print_help_readme_formatted();
            exit(EXIT_SUCCESS);
        }
    }

    print_help();
    exit(EXIT_SUCCESS);
}


void ArgParser::parse_generate_note() {
    if(cli_args.audio_input_method != DEFAULT_AUDIO_INPUT_METHOD) {
        std::string sample_getter_string;
        try {
            sample_getter_string = SampleGetterString.at(cli_args.audio_input_method);
        }
        catch(const std::out_of_range &e) {
            error("SampleGetter missing in SampleGetterString (in sample_getter/sample_getter.h)");
            exit(EXIT_FAILURE);
        }
        error("Can't generate a note while using '" + sample_getter_string + "' as input method");
        exit(EXIT_FAILURE);
    }

    cli_args.audio_input_method = SampleGetters::note_generator;

    const char *note_string;
    if(!fetch_opt(note_string))
        return;  // No more arguments, so use default value (set in config.h)

    try {
        cli_args.generate_note_note = string_to_note(note_string);
    }
    catch(const std::runtime_error &e) {
        error("Failed to parse note string '" + STR(note_string) + "': " + e.what());
        exit(EXIT_FAILURE);
    }
}


void ArgParser::parse_midi_out() {
    #ifdef NO_ALSA_MIDI
        error("Failed to enable MIDI output; ALSA support not compiled in");
        hint("Compile Digistring with ALSA MIDI support (see 'COMPILE_CONFIG' in the makefile)");
        exit(EXIT_FAILURE);
    #else
        cli_args.midi_out = true;
    #endif
}


void ArgParser::parse_output_file() {
    const char *filename;
    if(!fetch_opt(filename)) {
        filename = DEFAULT_OUTPUT_FILENAME.c_str();
        info("No file provided with output flag; using '" + STR(filename) + "' instead");
    }

    // Original filename as std::string instead of char[] for easier manipulation
    const std::string o_filename = filename;

    // Separate basename and extension
    std::string o_basename, o_extension;
    const size_t pos = o_filename.find_last_of('.');
    if(pos == std::string::npos || pos == 0)
        o_basename = o_filename;
    else {  // pos > 0
        o_basename = o_filename.substr(0, pos);
        o_extension = o_filename.substr(pos);
    }

    if(o_extension != ".json")
        warning("Extension of output file is not .json but '" + o_extension + "'");

    // Try to generate a new unique filename inserting a number between name and extension
    std::string g_filename = o_filename;  // Generated filename
    for(int i = 2; std::filesystem::exists(g_filename); i++)
        g_filename = o_basename + '_' + std::to_string(i) + o_extension;

    if(g_filename != o_filename)
        warning("File '" + o_filename + "' already exists; naming it '" + g_filename + "' instead");

    cli_args.output_file = true;
    cli_args.output_filename = g_filename;
}


void ArgParser::parse_print_overtone() {
    const char *note_string;
    if(!fetch_opt(note_string)) {
        error("No note provided with '--over' flag");
        exit(EXIT_FAILURE);
    }

    std::optional<Note> note;
    try {
        note = string_to_note(note_string);
    }
    catch(const std::runtime_error &e) {
        error("Failed to parse note string '" + std::string(note_string) + "': " + e.what());
        exit(EXIT_FAILURE);
    }
    // Getting here implies note was successfully initialized

    int n = 5;
    const char *n_string;
    if(fetch_opt(n_string)) {
        try {
            n = std::stoi(n_string);
        }
        catch(const std::out_of_range &e) {
            error("Number is too large to store in an integer");
            exit(EXIT_FAILURE);
        }
        catch(const std::exception &e) {
            error("Failed to parse '" + std::string(n_string) + "' as integer (" + STR(e.what()) + ")");
            exit(EXIT_FAILURE);
        }
    }

    const char *midi_cstring;
    if(fetch_opt(midi_cstring)) {
        const std::string midi_string = midi_cstring;
        if(midi_string == "midi_on")
            print_overtones(*note, n, true);
        else if(midi_string == "midi_off")
            print_overtones(*note, n, false);
        else {
            error("Invalid third argument passed... Either pass 'midi_on' or 'midi_off'");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }

    // Do the default if midi column isn't explicitly switched
    print_overtones(*note, n);
    exit(EXIT_SUCCESS);
}


void ArgParser::parse_playback() {
    const char *lr_cstring;
    if(fetch_opt(lr_cstring)) {
        const std::string lr_string = lr_cstring;
        if(lr_string == "left") {
            cli_args.stereo_split = true;
            cli_args.stereo_split_playback_left = true;
        }
        else if(lr_string == "right") {
            cli_args.stereo_split = true;
            cli_args.stereo_split_playback_left = false;
        }
        else {
            error("Invalid argument passed to playback flag; expected 'left' or 'right'");
            exit(EXIT_FAILURE);
        }
    }

    cli_args.playback = true;
}


void ArgParser::parse_play_note_event_file() {
    const char *note_event_file;
    if(!fetch_opt(note_event_file)) {
        error("No Digistring MIDI file provided");
        exit(EXIT_FAILURE);
    }

    cli_args.do_play_note_event_file = true;
    cli_args.note_event_file = note_event_file;

    // Select right synth if specified
    const char *synth_string;
    if(!fetch_opt(synth_string)) {
        error("No synth provided");
        exit(EXIT_FAILURE);
    }
    try {
        cli_args.synth_type = parse_synth_string.at(synth_string);
    }
    catch(const std::out_of_range &e) {
        error("Unknown synth type '" + std::string(synth_string) + "'");
        exit(EXIT_FAILURE);
    }

    const char *out_dev_arg;
    if(fetch_opt(out_dev_arg))
        cli_args.out_dev_name = out_dev_arg;
}


void ArgParser::parse_print_performance() {
    const char *perf_file;
    if(!fetch_opt(perf_file)) {
        cli_args.output_performance = true;
        return;
    }
    else {
        cli_args.perf_output_file = perf_file;
    }
}


void ArgParser::parse_resolution() {
    const char *w_string, *h_string;
    if(!fetch_opt(w_string)) {
        error("No width provided with '-r' flag");
        exit(EXIT_FAILURE);
    }
    if(!fetch_opt(h_string)) {
        error("No height provided with '-r' flag");
        exit(EXIT_FAILURE);
    }

    int n;
    try {
        n = std::stoi(w_string);
    }
    catch(const std::out_of_range &e) {
        error("Width is too large to store in an integer");
        exit(EXIT_FAILURE);
    }
    catch(const std::exception &e) {
        error("Failed to parse given width as an integer (" + STR(e.what()) + ")");
        exit(EXIT_FAILURE);
    }
    if(n < MIN_RES[0]) {
        error("Width is too small (" + STR(n) + " < " + STR(MIN_RES[0]) + ")");
        exit(EXIT_FAILURE);
    }
    cli_args.res_w = n;

    try {
        n = std::stoi(h_string);
    }
    catch(const std::out_of_range &e) {
        error("Height is too large to store in an integer");
        exit(EXIT_FAILURE);
    }
    catch(const std::exception &e) {
        error("Failed to parse given height as an integer (" + STR(e.what()) + ")");
        exit(EXIT_FAILURE);
    }
    if(n < MIN_RES[1]) {
        error("Height is too small (" + STR(n) + " < " + STR(MIN_RES[1]) + ")");
        exit(EXIT_FAILURE);
    }
    cli_args.res_h = n;
}


void ArgParser::parse_rsc_dir() {
    const char *path_string;
    if(!fetch_opt(path_string)) {
        error("No path provided with '--rsc' flag");
        exit(EXIT_FAILURE);
    }

    cli_args.rsc_dir = path_string;
}


void ArgParser::parse_generate_sine() {
    if(cli_args.audio_input_method != DEFAULT_AUDIO_INPUT_METHOD) {
        std::string sample_getter_string;
        try {
            sample_getter_string = SampleGetterString.at(cli_args.audio_input_method);
        }
        catch(const std::out_of_range &e) {
            error("SampleGetter missing in SampleGetterString (in sample_getter/sample_getter.h)");
            exit(EXIT_FAILURE);
        }
        error("Can't generate sine while using '" + sample_getter_string + "' as input method");
        exit(EXIT_FAILURE);
    }

    cli_args.audio_input_method = SampleGetters::wave_generator;

    const char *arg;
    if(!fetch_opt(arg))  // No more arguments, so use default value (set in config.h)
        return;

    double f;
    try {
        f = std::stod(arg);
    }
    catch(const std::out_of_range &e) {
        error("Given frequency cannot be represented by a double");
        exit(EXIT_FAILURE);
    }
    catch(const std::exception &e) {
        error("Failed to parse given frequency as a floating point number (" + STR(e.what()) + ")");
        exit(EXIT_FAILURE);
    }
    if(f < 1.0) {
        error("Can't set frequency below 1 Hz (got " + STR(f) + ")");
        exit(EXIT_FAILURE);
    }
    cli_args.generate_sine_freq = f;
}


void ArgParser::parse_slow() {
    const char *arg;
    if(!fetch_opt(arg))  // No more arguments, so use default value (set in config.h)
        return;

    double s;
    try {
        s = std::stod(arg);
    }
    catch(const std::out_of_range &e) {
        error("Slowdown factor cannot be represented by a double");
        exit(EXIT_FAILURE);
    }
    catch(const std::exception &e) {
        error("Failed to parse given slowdown factor as a floating point number (" + STR(e.what()) + ")");
        exit(EXIT_FAILURE);
    }
    if(s <= 1.0) {
        error("Can't set slowdown factor <= 1.0");
        exit(EXIT_FAILURE);
    }

    cli_args.do_slowdown = true;
    cli_args.slowdown_factor = s;
}


void ArgParser::parse_sync_with_audio() {
    // Program class may overwrite this value if SampleGetter is blocking
    cli_args.sync_with_audio = true;
}


void ArgParser::parse_synth() {
    cli_args.synth = true;

    // Select right synth if specified
    const char *synth_string;
    if(!fetch_opt(synth_string))
        return;  // Default is set in config.cli_args.h

    try {
        cli_args.synth_type = parse_synth_string.at(synth_string);
    }
    catch(const std::out_of_range &e) {
        error("Unknown synth type '" + std::string(synth_string) + "'");
        exit(EXIT_FAILURE);
    }

    const char *volume_string;
    if(!fetch_opt(volume_string))  // No more arguments, so use default value (set in config.h)
        return;

    double volume;
    try {
        volume = std::stod(volume_string);
    }
    catch(const std::out_of_range &e) {
        error("Given volume cannot be represented by a double");
        exit(EXIT_FAILURE);
    }
    catch(const std::exception &e) {
        error("Failed to parse given volume as a floating point number (" + STR(e.what()) + ")");
        exit(EXIT_FAILURE);
    }
    if(volume < 0.0) {
        error("Can't set volume below 0.0 (got " + STR(volume) + ")");
        exit(EXIT_FAILURE);
    }
    if(volume > 1.0) {
        error("Can't set volume above 1.0 (got " + STR(volume) + ")");
        exit(EXIT_FAILURE);
    }

    cli_args.volume = volume;
}


void ArgParser::parse_synths() {
    std::cout << "Available synthesizers:" << std::endl;
    for(const auto &[key, value] : parse_synth_string) {
        try {
            std::string description = synth_description.at(value);
            std::cout << "  - " << key << ": " << description << std::endl;
        }
        catch(const std::out_of_range &e) {
            std::cout << "  - " << key << std::endl;
        }
    }

    exit(EXIT_SUCCESS);
}
