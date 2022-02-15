
#include "quit.h"

#include "error.h"

#include <string>
#include <cstring>  // sigabbrev_np()


static volatile bool quit;

void signal_handler(const int signum) {
    if(quit) {
        info("Received signal 'SIG" + std::string(sigabbrev_np(signum)) + "' while quitting; will now force quit");
        exit(-2);
    }

    std::cout << std::endl;
    info("Signal 'SIG" + std::string(sigabbrev_np(signum)) + "' received; quitting...");

    set_quit();
}


bool poll_quit() {
    return quit;
}

void set_quit() {
    info("Quitting application...");
    quit = true;
}

void reset_quit() {
    quit = false;
}
