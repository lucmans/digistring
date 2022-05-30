#ifndef ERROR_H
#define ERROR_H


#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>

#include <unistd.h>  // isatty()


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


// Without file and line number
inline void __msg(const char *type, const char *color, const std::string &msg) {
    if(isatty(STDERR_FILENO) == 1) {
        std::cerr << BOLD << color << type << RESET << ": " << msg << std::endl;
    }
    else {
        std::cerr << type << ": " << msg << std::endl;
    }
}

// With file and line number
inline void __msg(const char *type, const char *color, const char *file, const int line, const std::string &msg) {
    if(isatty(STDERR_FILENO) == 1) {
        std::cerr << BOLD << color << type << RESET << ": " << msg << " (" << file << ":" << line << ")" << std::endl;
    }
    else {
        std::cerr << type << ": " << msg << " (" << file << ":" << line << ")" << std::endl;
    }
}

// Plain messages
inline void __msg(const std::string &msg) {
    std::cerr << msg << std::endl;
}


inline std::string __str(const char *x) {
    return std::string(x);
}

template <typename T>
std::string __str(T x) {
    return std::to_string(x);
}

#define STR(x) __str(x)

#define error(msg)    __msg("Error", RED, __FILE__, __LINE__, (msg))
#define warning(msg)  __msg("Warning", YELLOW, __FILE__, __LINE__, (msg))
#define info(msg)     __msg("Info", GREEN, (msg))
#define debug(msg)    __msg("Debug", BLUE, __FILE__, __LINE__, (msg))
#define hint(msg)     __msg("Hint", BLUE, __FILE__, __LINE__, (msg))


#endif  // ERROR_H
