
#ifndef ERROR_H
#define ERROR_H


// #include "performance.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
// #include <unistd.h>  // isatty
#include <stdexcept>


// Uncomment relevant lines; commented to improve compiling times, as printing times relies on performance, which relies on config
const bool TIMES = false;  // Display timestamp in CLI messages

const char GREEN[] = "\033[32m";
const char BLUE[] = "\033[34m";
const char YELLOW[] = "\033[33m";
const char RED[] = "\033[31m";
const char BOLD[] = "\033[1m";
const char RESET[] = "\033[0m";


// Exception message
inline std::string __ex_msg(const char *file, int line, const std::string &msg) {
    std::string out_msg = "";
    out_msg += RED;
    out_msg += BOLD;
    // if(TIMES) {
    //     out_msg += "[";
    //     out_msg += perf.get_program_time();
    //     out_msg += "] ";
    // }

    out_msg += "Unhandled video exception: ";
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
    std::cout << GREEN << BOLD;
    // if(TIMES)
    //     std::cout << "[" << perf.get_program_time() << "] ";
    std::cout << "Info" << RESET << ": " << msg << std::endl;
}

inline void __msg(const char *type, const char *color, const char *file, const int line, const std::string &msg) {
    std::cerr << color << BOLD;
    // if(TIMES)
    //     std::cerr << "[" << perf.get_program_time() << "] ";
    std::cerr << type << RESET << ": " << msg << " (" << file << ":" << line << ")" << std::endl;
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


inline void ex_msg(const std::string &msg) {
    __ex_msg(__FILE__, __LINE__, (msg));
}

inline void error(const std::string &msg) {
    __msg("Error", RED, __FILE__, __LINE__, (msg));
}

inline void warning(const std::string &msg) {
    __msg("Warning", YELLOW, __FILE__, __LINE__, (msg));
}

inline void info(const std::string &msg) {
    // __msg("Info", GREEN, __FILE__, __LINE__, (msg));
    __info_msg((msg));  // Prints to std::cout and does not print file:line
}

inline void debug(const std::string &msg) {
    __msg("Debug", BLUE, __FILE__, __LINE__, (msg));
}


#endif  // ERROR_H
