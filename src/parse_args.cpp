
#include "parse_args.h"

#include "config.h"
#include "error.h"

#include "note.h"

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <string>  // std::stoi()
#include <filesystem>  // basic checks on given rsc directory
#include <functional>  // std::invoke()
#include <optional>  // Construct note without initializing it


#include <map>

const std::map<std::string, void (ArgParser::*const)()> ArgParser::flag_to_func = {
    {"-f", &ArgParser::parse_fullscreen},
    {"--file", &ArgParser::parse_file},
    {"-h", &ArgParser::parse_help},
    {"--help", &ArgParser::parse_help},
    {"-n", &ArgParser::parse_generate_note},
    {"--over", &ArgParser::parse_print_overtone},
    {"-p", &ArgParser::parse_playback},
    {"--perf", &ArgParser::parse_print_performance},
    {"-r", &ArgParser::parse_resolution},
    {"--rsc", &ArgParser::parse_rsc_dir},
    {"-s", &ArgParser::parse_generate_sine}
};

void print_help() {
    std::cout << "Flags:\n"
              << "  -f                - Start in fullscreen. Also set the fullscreen resolution with '-r'\n"
              << "  --file <file>     - Play samples from given file\n"
              << "  -n [note]         - Generate note (default is A4)\n"
              << "  --over <note> [n] - Print n (default is 5) overtones of given note\n"
              << "  -p                - Play recorded audio back\n"
              << "  --perf            - Output performance stats in stdout\n"
              << "  -r <w> <h>        - Start GUI with given resolution\n"
              << "  --rsc <path>      - Set alternative resource directory location\n"
              << "  -s [f]            - Generate sine wave with optional frequency f (default is 1000 Hz) instead of using recording device\n"
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
    settings.fullscreen = true;
}

void ArgParser::parse_file() {
    const char *arg;
    if(!fetch_arg(arg)) {
        error("No file given with the --file flag");
        exit(EXIT_FAILURE);
    }

    if(settings.generate_note) {
        error("Can't play a file while generating a note");
        exit(EXIT_FAILURE);
    }
    if(settings.generate_sine) {
        error("Can't play a file while generating a sine wave");
        exit(EXIT_FAILURE);
    }

    if(!std::filesystem::exists(arg)) {
        error("Given file does not exist");
        exit(EXIT_FAILURE);
    }

    settings.play_file = true;
    settings.play_file_name = arg;
}

void ArgParser::parse_help() {
    print_help();
    exit(EXIT_SUCCESS);
}

void ArgParser::parse_generate_note() {
    if(settings.play_file) {
        error("Can't generate a note while playing a file");
        exit(EXIT_FAILURE);
    }
    if(settings.generate_sine) {
        error("Can't generate a note while generating a sine wave");
        exit(EXIT_FAILURE);
    }

    settings.generate_note = true;

    const char *arg;
    if(!fetch_opt(arg))  // No more arguments, so use default value (set in config.h)
        return;

    try {
        settings.generate_note_note = string_to_note(arg);
    }
    catch(const std::string &what) {
        error("Failed to parse note string: " + what);
        exit(EXIT_FAILURE);
    }
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
    settings.playback = true;
}

void ArgParser::parse_print_performance() {
    settings.output_performance = true;
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
    settings.res_w = n;

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
    settings.res_h = n;
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

    settings.rsc_dir = path_string;
}

void ArgParser::parse_generate_sine() {
    if(settings.play_file) {
        error("Can't generate sine wave while playing a file");
        exit(EXIT_FAILURE);
    }
    if(settings.generate_note) {
        error("Can't generate sine wave while generating a note");
        exit(EXIT_FAILURE);
    }

    settings.generate_sine = true;

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
    settings.generate_sine_freq = f;
}