#ifndef PARSE_CLI_ARGS_H
#define PARSE_CLI_ARGS_H


#include <map>
#include <string>
#include <functional>


// Struct with all settings that can be changed through CLI arguments
// Should only be written to by CLIArgParser at the start of the program
struct CLIArgs {
    int res_w = -1;
    int res_h = -1;

    // Audio device settings
    // Empty string will force passing NULL to let SDL select the best choice
    std::string in_dev_name = "";
    std::string out_dev_name = "";
};
extern CLIArgs cli_args;


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
        void parse_help();
        void parse_resolution();
};


// Struct holding the parse function and OptTypes
struct ParseObj {
    const std::function<void(ArgParser&)> function;

    ParseObj(const std::function<void(ArgParser&)> _function) : function(_function) {};
};


#endif  // PARSE_CLI_ARGS_H
