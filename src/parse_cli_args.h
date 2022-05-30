#ifndef DIGISTRING_PARSE_CLI_ARGS_H
#define DIGISTRING_PARSE_CLI_ARGS_H


#include <map>
#include <string>
#include <functional>


// Prototype before ArgParser
class ParseObj;

// Declare the function pointer to the compare function as compare_func_t
typedef bool(*compare_func_t)(const std::string,const std::string);


// Helps hiding creating an ArgParser object
void parse_args(const int argc, const char *const argv[]);

class ArgParser {
    public:
        ArgParser(const int argc, const char *const argv[]);
        ~ArgParser();

        // Returns true if another argument was fetched
        bool fetch_arg(const char *&arg);

        // Return true if a non flag (not starting with '-') argument was fetched
        bool fetch_opt(const char *&arg);

        void parse_args();

        // In generate_completions.cpp
        void generate_completions();


    private:
        // Holds what index in argv will be returned by the next fetch_arg() call
        int cur_arg;

        const int argc;
        const char *const *const argv;  // TODO: Conform constructor

        // Maps flag strings to functions which parse the flag
        // static const std::map<const std::string, const ParseObj> flag_to_func;
        static const std::map<const std::string, const ParseObj, compare_func_t> flag_to_func;


        void parse_audio();
        void parse_audio_in();
        void parse_audio_out();
        void parse_fullscreen();
        void parse_file();
        void parse_help();
        void parse_generate_note();
        void parse_output_file();
        void parse_print_overtone();
        void parse_playback();
        void parse_print_performance();
        void parse_resolution();
        void parse_rsc_dir();
        void parse_generate_sine();
        void parse_sync_with_audio();
        void parse_synth();
        void parse_synths();
};


/* For automatically generating completions */
// The OptType determines what kind of tab complete to perform on arguments of flags
// last_arg will prevent further completions to be given (useful for signalling no other flags are possible)
enum class OptType {
    dir, file, output_file, completions_file, opt_decimal, integer, opt_integer, note, opt_note, last_arg, opt_synth, opt_left_right, audio_in_device, audio_out_device, midi_switch
};

// Struct holding the parse function and OptTypes
struct ParseObj {
    const std::function<void(ArgParser&)> function;
    const std::vector<OptType> opt_types;

    ParseObj(const std::function<void(ArgParser&)> _function, const std::vector<OptType> &_opt_types) :
            function(_function),
            opt_types(_opt_types) {};
};


#endif  // DIGISTRING_PARSE_CLI_ARGS_H
