#include "quit.h"

#include "error.h"

#include <csignal>  // catching signals
#include <cstring>  // sigabbrev_np()
#include <execinfo.h>  // backtrace() functions


static volatile bool quit = false;

bool poll_quit() {
    return quit;
}

void set_quit() {
    info("Quitting application on next cycle...");
    quit = true;
}

void reset_quit() {
    quit = false;
}


void set_signal_handlers() {
    signal(SIGINT, quit_signal_handler);
    signal(SIGTERM, quit_signal_handler);

    signal(SIGSEGV, backtrace_quit_signal_handler);
}

void quit_signal_handler(const int signum) {
    if(quit) {
        info("Received signal 'SIG" + STR(sigabbrev_np(signum)) + "' while quitting; will now force quit");
        exit(-2);
    }

    __msg("");  // Print on a new line for when using ctrl+c to send a SIGINT (causes ^C to be printed)
    info("Signal 'SIG" + STR(sigabbrev_np(signum)) + "' received");

    set_quit();
}


void backtrace_quit_signal_handler(const int signum) {
    if(signum == SIGSEGV)
        error("Segmentation fault occurred; printing stack trace (use addr2line to resolve offsets)...");
    else
        error("Signal 'SIG" + STR(sigabbrev_np(signum)) + "' received; printing stack trace (use addr2line to resolve offsets)...");
    hint("First two in call stack are likely from signal handler");

    const int n_frames = 10;
    void *frames[n_frames];
    int size;
    size = backtrace(frames, n_frames);
    // for(int i = 0; i < n_frames; i++)
    //     std::cout << frames[i] << std::endl;

    char **strings = backtrace_symbols(frames, size);
    if(strings != NULL) {
        for(int i = 0; i < size; i++)
            std::cout << i + 1 << ": " << strings[i] << std::endl;
    }

    // TODO: Use dladdr1() and/or abi::__cxa_demangle() for more information

    free(strings);
    exit(EXIT_FAILURE);
}
