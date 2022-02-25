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


constexpr bool TIMES = false;  // Display timestamp in CLI messages

const char GREEN[] = "\033[32m";
const char BLUE[] = "\033[34m";
const char YELLOW[] = "\033[33m";
const char RED[] = "\033[31m";
const char BOLD[] = "\033[1m";
const char RESET[] = "\033[0m";


// Exception message
inline std::string __ex_msg(const char *file, int line, const std::string &msg) {
    std::string out_msg = "";
    out_msg += BOLD;
    if constexpr(TIMES) {
        out_msg += "[";
        out_msg += perf.get_program_time();
        out_msg += "] ";
    }

    out_msg += RED;
    out_msg += "Unhandled exception: ";
    out_msg += RESET;
    out_msg += msg;
    out_msg += " (";
    out_msg += file;
    out_msg += ":";
    out_msg += line;
    out_msg += ")";

    return out_msg;
}


inline void __info_msg(const std::string msg) {
    std::cout << BOLD;
    if constexpr(TIMES)
        std::cout << "[" << perf.get_program_time() << "] ";

    std::cout << GREEN << "Info" << RESET << ": " << msg << std::endl;
}

inline void __msg(const char *type, const char *color, const char *file, const int line, const std::string &msg) {
    std::cerr << BOLD;
    if constexpr(TIMES)
        std::cerr << "[" << perf.get_program_time() << "] ";

    std::cerr << color << type << RESET << ": " << msg << " (" << file << ":" << line << ")" << std::endl;
}


// class CustomExcept : public std::runtime_error {
//     public:
//         CustomExcept(const std::string &msg) : std::runtime_error(msg) {};
// };


inline std::string __str(const char *x) {
    return std::string(x);
}

template <typename T>
std::string __str(T x) {
    return std::to_string(x);
}

#define STR(x)              __str(x)


inline void ex_msg(const std::string &msg, const std::source_location location = std::source_location::current()) {
    __ex_msg(location.file_name(), location.line(), msg);
}

inline void error(const std::string &msg, const std::source_location location = std::source_location::current()) {
    __msg("Error", RED, location.file_name(), location.line(), msg);
}

inline void warning(const std::string &msg, const std::source_location location = std::source_location::current()) {
    __msg("Warning", YELLOW, location.file_name(), location.line(), msg);
}

// inline void info(const std::string &msg, const std::source_location location = std::source_location::current()) {
//     __msg("Info", GREEN, location.file_name(), location.line(), msg);
// }
inline void info(const std::string &msg) {
    __info_msg(msg);  // Prints to std::cout and does not print file:line
}

inline void debug(const std::string &msg, const std::source_location location = std::source_location::current()) {
    __msg("Debug", BLUE, location.file_name(), location.line(), msg);
}


#endif  // DIGISTRING_ERROR_H
