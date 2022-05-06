#ifndef DIGISTRING_ERROR_H
#define DIGISTRING_ERROR_H


#include "performance.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>
#include <source_location>

#include <unistd.h>  // isatty()


// Print time from program start in CLI messages
#ifdef MSG_PRINT_TIME
    constexpr bool PRINT_PROGRAM_TIME = true;
#else
    constexpr bool PRINT_PROGRAM_TIME = false;
#endif  // MSG_PRINT_TIME


#ifndef NO_COLORS
    const char BLACK[]     = "\033[30m";
    const char RED[]       = "\033[31m";
    const char GREEN[]     = "\033[32m";
    const char YELLOW[]    = "\033[33m";
    const char BLUE[]      = "\033[34m";
    const char MAGENTA[]   = "\033[35m";
    const char CYAN[]      = "\033[36m";
    const char WHITE[]     = "\033[37m";
    const char BOLD[]      = "\033[1m";
    const char ITALIC[]    = "\033[3m";
    const char UNDERLINE[] = "\033[4m";
    const char RESET[]     = "\033[0m";
#else
    const char BLACK[]     = "";
    const char RED[]       = "";
    const char GREEN[]     = "";
    const char YELLOW[]    = "";
    const char BLUE[]      = "";
    const char MAGENTA[]   = "";
    const char CYAN[]      = "";
    const char WHITE[]     = "";
    const char BOLD[]      = "";
    const char ITALIC[]    = "";
    const char UNDERLINE[] = "";
    const char RESET[]     = "";
#endif  // NO_COLORS


// // Exception message
// inline std::string __ex_msg(const char *file, int line, const std::string &msg) {
//     std::string out_msg = "";
//     out_msg += BOLD;
//     if constexpr(PRINT_PROGRAM_TIME) {
//         out_msg += "[";
//         out_msg += perf.get_program_time();
//         out_msg += "] ";
//     }

//     out_msg += RED;
//     out_msg += "Unhandled exception: ";
//     out_msg += RESET;
//     out_msg += msg;
//     out_msg += " (";
//     out_msg += file;
//     out_msg += ":";
//     out_msg += line;
//     out_msg += ")";

//     return out_msg;
// }

// inline void ex_msg(const std::string &msg, const std::source_location location = std::source_location::current()) {
//     __ex_msg(location.file_name(), location.line(), msg);
// }


// class CustomExcept : public std::runtime_error {
//     public:
//         CustomExcept(const std::string &msg) : std::runtime_error(msg) {};
// };


// Without file and line number
inline void __msg(const char *type, const char *color, const std::string &msg) {
    if(isatty(STDERR_FILENO)) {
        std::cerr << BOLD;
        if constexpr(PRINT_PROGRAM_TIME)
            std::cerr << "[" << perf.get_program_time() << "] ";

        std::cerr << color << type << RESET << ": " << msg << std::endl;
    }
    else {
        if constexpr(PRINT_PROGRAM_TIME)
            std::cerr << "[" << perf.get_program_time() << "] ";

        std::cerr << type << ": " << msg << std::endl;
    }
}

// With file and line number
inline void __msg(const char *type, const char *color, const char *file, const int line, const std::string &msg) {
    if(isatty(STDERR_FILENO)) {
        std::cerr << BOLD;
        if constexpr(PRINT_PROGRAM_TIME)
            std::cerr << "[" << perf.get_program_time() << "] ";

        std::cerr << color << type << RESET << ": " << msg << " (" << file << ":" << line << ")" << std::endl;
    }
    else {
        if constexpr(PRINT_PROGRAM_TIME)
            std::cerr << "[" << perf.get_program_time() << "] ";

        std::cerr << type << ": " << msg << " (" << file << ":" << line << ")" << std::endl;
    }
}


inline std::string __str(const char *x) {
    return std::string(x);
}

template <typename T>
std::string __str(T x) {
    return std::to_string(x);
}

#define STR(x) __str(x)

inline void error(const std::string &msg, const std::source_location location = std::source_location::current()) {
    __msg("Error", RED, location.file_name(), location.line(), msg);
}

inline void warning(const std::string &msg, const std::source_location location = std::source_location::current()) {
    __msg("Warning", YELLOW, location.file_name(), location.line(), msg);
}

#ifdef INFO_SOURCE_LOC
    inline void info(const std::string &msg, const std::source_location location = std::source_location::current()) {
        __msg("Info", GREEN, location.file_name(), location.line(), msg);
    }
#else
    inline void info(const std::string &msg) {
        __msg("Info", GREEN, msg);
    }
#endif  // INFO_SOURCE_LOC

inline void debug(const std::string &msg, const std::source_location location = std::source_location::current()) {
    __msg("Debug", BLUE, location.file_name(), location.line(), msg);
}

inline void hint(const std::string &msg) {
    __msg("Hint", BLUE, msg);
}


#endif  // DIGISTRING_ERROR_H
