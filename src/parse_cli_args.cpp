
#include "parse_cli_args.h"

#include "note.h"
#include "error.h"

#include "config/cli_args.h"
#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"
#include "config/results_file.h"

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <string>  // std::stoi()
#include <filesystem>  // basic checks on given rsc directory
#include <functional>  // std::invoke()
#include <optional>  // Construct note without initializing it
#include <map>


CLIArgs cli_args;

const std::map<const std::string, void (ArgParser::*const)()> ArgParser::flag_to_func = {
    {"-f", &ArgParser::parse_fullscreen},
    {"--file", &ArgParser::parse_file},
    {"-h", &ArgParser::parse_help},
    {"--help", &ArgParser::parse_help},
    {"-n", &ArgParser::parse_generate_note},
    {"-o", &ArgParser::parse_output_file},
    {"--output", &ArgParser::parse_output_file},
    {"--over", &ArgParser::parse_print_overtone},
    {"-p", &ArgParser::parse_playback},
    {"--perf", &ArgParser::parse_print_performance},
    {"-r", &ArgParser::parse_resolution},
    {"--rsc", &ArgParser::parse_rsc_dir},
    {"-s", &ArgParser::parse_generate_sine}
};

void print_help() {
    std::cout << "Flags (argument parameters in <> are required and in [] are optional):\n"
              << "  -f                   - Start in fullscreen. Also set the fullscreen resolution with '-r'\n"
              << "  --file <file>        - Play samples from given file\n"
              << "  -n [note]            - Generate note (default is A4)\n"
              << "  -o | --output [file] - Write estimation results as JSON to file (default filename is output.json)\n"
              << "  --over <note> [n]    - Print n (default is 5) overtones of given note\n"
              << "  -p                   - Play recorded audio back\n"
              << "  --perf               - Output performance stats in stdout\n"
              << "  -r <w> <h>           - Start GUI with given resolution\n"
              << "  --rsc <path>         - Set alternative resource directory location\n"
              << "  -s [f]               - Generate sine wave with optional frequency f (default is 1000 Hz) instead of using recording device\n"
              << std::endl;
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
            std::invoke(flag_to_func.at(arg), this);
        }
        catch(const std::out_of_range &oor) {
            std::cout << "Incorrect usage; flag '" + std::string(arg) + "' not known\n" << std::endl;
            print_help();
            exit(EXIT_FAILURE);
        }
        catch(...) {
            error("Failed to call function associated with given flag");
            exit(EXIT_FAILURE);
        }
    }
}


void ArgParser::parse_fullscreen() {
    cli_args.fullscreen = true;
}

void ArgParser::parse_file() {
    const char *arg;
    if(!fetch_arg(arg)) {
        error("No file given with the --file flag");
        exit(EXIT_FAILURE);
    }

    if(cli_args.generate_note) {
        error("Can't play a file while generating a note");
        exit(EXIT_FAILURE);
    }
    if(cli_args.generate_sine) {
        error("Can't play a file while generating a sine wave");
        exit(EXIT_FAILURE);
    }

    if(!std::filesystem::exists(arg)) {
        error("Given file does not exist");
        exit(EXIT_FAILURE);
    }

    cli_args.play_file = true;
    cli_args.play_file_name = arg;
}

void ArgParser::parse_help() {
    print_help();
    exit(EXIT_SUCCESS);
}

void ArgParser::parse_generate_note() {
    if(cli_args.play_file) {
        error("Can't generate a note while playing a file");
        exit(EXIT_FAILURE);
    }
    if(cli_args.generate_sine) {
        error("Can't generate a note while generating a sine wave");
        exit(EXIT_FAILURE);
    }

    cli_args.generate_note = true;

    const char *arg;
    if(!fetch_opt(arg))  // No more arguments, so use default value (set in config.h)
        return;

    try {
        cli_args.generate_note_note = string_to_note(arg);
    }
    catch(const std::string &what) {
        error("Failed to parse note string: " + what);
        exit(EXIT_FAILURE);
    }
}

void ArgParser::parse_output_file() {
    const char *filename;
    if(!fetch_opt(filename)) {
        filename = DEFAULT_OUTPUT_FILENAME;
        info("No file provided with output flag; using '" + STR(filename) + " instead");
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
        warning("Extension of output file is not '.json' " + o_extension);

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
    catch(const std::string &what) {
        error("Failed to parse note string '" + std::string(note_string) + "': " + what);
        exit(EXIT_FAILURE);
    }

    int n = 5;
    const char *n_string;
    if(fetch_opt(n_string)) {
        try {
            n = std::stoi(n_string);
        }
        catch(...) {
            error("Failed to parse '" + std::string(n_string) + "' as integer");
            exit(EXIT_FAILURE);
        }
    }

    print_overtones(*note, n);  // Getting here implies note was successfully initialized
    exit(EXIT_SUCCESS);
}

void ArgParser::parse_playback() {
    cli_args.playback = true;

    if constexpr(DO_OVERLAP || DO_OVERLAP_NONBLOCK) {
        const char *overlap;
        if(!fetch_opt(overlap)) {
            error("Playing audio back while using overlapping frames will cause distorted audio.\n"
                  "Pass 'overlap' to the '-p' flag to overwrite this warning or disable DO_OVERLAP in config.h.");
            exit(EXIT_FAILURE);
        }

        if(strncmp(overlap, "overlap", 7) != 0) {
            error("Unknown option '" + std::string(overlap) + "' for -p flag");
            exit(EXIT_FAILURE);
        }
    }
}

void ArgParser::parse_print_performance() {
    cli_args.output_performance = true;
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
    catch(...) {
        error("Failed to parse given width '" + std::string(w_string) + "'");
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
    catch(...) {
        error("Failed to parse given height '" + std::string(h_string) + "'");
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

    std::filesystem::path path(path_string);
    path = path.lexically_normal();

    try {
        path = std::filesystem::canonical(path);
    }
    catch(...) {
        error("Path '" + std::string(path_string) + "' doesn't exist");
        exit(EXIT_FAILURE);
    }

    // Get last part of path
    std::filesystem::path::iterator it = path.end();
    --it;
    if(*it == "")  // Is empty if path ends with a /
        --it;

    if(*it != "rsc") {
        error("Last part of path is not named 'rsc'");
        exit(EXIT_FAILURE);
    }

    cli_args.rsc_dir = path_string;
}

void ArgParser::parse_generate_sine() {
    if(cli_args.play_file) {
        error("Can't generate sine wave while playing a file");
        exit(EXIT_FAILURE);
    }
    if(cli_args.generate_note) {
        error("Can't generate sine wave while generating a note");
        exit(EXIT_FAILURE);
    }

    cli_args.generate_sine = true;

    const char *arg;
    if(!fetch_opt(arg))  // No more arguments, so use default value (set in config.h)
        return;

    double f;
    try {
        f = std::stod(arg);
    }
    catch(...) {
        error("Failed to parse frequency '" + std::string(arg) + "'");
        exit(EXIT_FAILURE);
    }
    if(f < 1.0) {
        error("Frequency below 1 Hz (" + STR(f) + ")");
        exit(EXIT_FAILURE);
    }
    cli_args.generate_sine_freq = f;
}
