#include "startup_timer.h"

#include "error.h"

#include <chrono>


StartupTimer startup_timer;


StartupTimer::StartupTimer() {
    start_program = std::chrono::steady_clock::now();
    init_time = -1;
}

StartupTimer::~StartupTimer() {

}


double StartupTimer::get_program_time() const {
    std::chrono::duration<double, std::milli> dur = std::chrono::steady_clock::now() - start_program;

    return dur.count();
}


double StartupTimer::get_init_time() const {
    if(init_time < 0) {
        warning("Tried to get init time before setting init time");
        return -1;
    }

    return init_time;
}

void StartupTimer::set_init_time() {
    // Only allow being called once
    static bool called = false;
    if(called) {
        warning("StartupTimer::set_init_time() is called again");
        return;
    }
    called = true;

    std::chrono::duration<double, std::milli> t = std::chrono::steady_clock::now() - start_program;
    init_time = t.count();
}
