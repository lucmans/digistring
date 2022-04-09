#include "quit.h"

#include "error.h"

#include <iostream>
#include <string>
#include <cstring>  // sigabbrev_np()


static volatile bool quit;

void signal_handler(const int signum) {
    if(quit) {
        info("Received signal 'SIG" + STR(sigabbrev_np(signum)) + "' while quitting; will now force quit");
        exit(-2);
    }

    std::cout << std::endl;
    info("Signal 'SIG" + STR(sigabbrev_np(signum)) + "' received");

    set_quit();
}


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
