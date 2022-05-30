#include "parse_cli_args.h"

#include "init_sdl_audio.h"
// #include "config.h"
#include "error.h"

#include <SDL2/SDL.h>  // Init SDL for checking audio devices

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <string>  // std::stoi(), std::stod()
#include <functional>  // std::invoke()
#include <map>


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
        {"--audio",             ParseObj(&ArgParser::parse_audio)},
        {"--audio_in",          ParseObj(&ArgParser::parse_audio_in)},
        {"--audio_out",         ParseObj(&ArgParser::parse_audio_out)},
        {"-h",                  ParseObj(&ArgParser::parse_help)},
        {"--help",              ParseObj(&ArgParser::parse_help)},
        // {"-r",                  ParseObj(&ArgParser::parse_resolution)},
    },
    flag_ordering
};


const std::pair<const std::string, const std::string> help_strings[] = {
    {"--audio",                     "Print used audio driver and available audio devices"},
    {"--audio_in <device name>",    "Set the recording device to device name (as provided by Digistring at start-up"},
    {"--audio_out <device name>",   "Set the playback device to device name (as provided by Digistring at start-up"},
    {"-h | --help",                 "Print command line argument information. Optionally pass 'readme' for readme formatting"},
    // {"-r <w> <h>",                  "Start GUI with given resolution"},
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
            error("Incorrect usage; flag '" + std::string(arg) + "' not known\n");
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


// void ArgParser::parse_resolution() {
//     const char *w_string, *h_string;
//     if(!fetch_opt(w_string)) {
//         error("No width provided with '-r' flag");
//         exit(EXIT_FAILURE);
//     }
//     if(!fetch_opt(h_string)) {
//         error("No height provided with '-r' flag");
//         exit(EXIT_FAILURE);
//     }

//     int n;
//     try {
//         n = std::stoi(w_string);
//     }
//     catch(const std::out_of_range &e) {
//         error("Width is too large to store in an integer");
//         exit(EXIT_FAILURE);
//     }
//     catch(const std::exception &e) {
//         error("Failed to parse given width as an integer (" + STR(e.what()) + ")");
//         exit(EXIT_FAILURE);
//     }
//     if(n < MIN_RES[0]) {
//         error("Width is too small (" + STR(n) + " < " + STR(MIN_RES[0]) + ")");
//         exit(EXIT_FAILURE);
//     }
//     cli_args.res_w = n;

//     try {
//         n = std::stoi(h_string);
//     }
//     catch(const std::out_of_range &e) {
//         error("Height is too large to store in an integer");
//         exit(EXIT_FAILURE);
//     }
//     catch(const std::exception &e) {
//         error("Failed to parse given height as an integer (" + STR(e.what()) + ")");
//         exit(EXIT_FAILURE);
//     }
//     if(n < MIN_RES[1]) {
//         error("Height is too small (" + STR(n) + " < " + STR(MIN_RES[1]) + ")");
//         exit(EXIT_FAILURE);
//     }
//     cli_args.res_h = n;
// }
