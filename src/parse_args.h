
#ifndef PARSE_ARGS_H
#define PARSE_ARGS_H


#include <map>
#include <string>


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


    private:
        int cur_arg;  // Holds what index in argv will be returned by the next fetch_arg() call

        const int argc;
        const char *const *const argv;


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

        static const std::map<const std::string, void (ArgParser::*const)()> flag_to_func;
};


#endif  // PARSE_ARGS_H
