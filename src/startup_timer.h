#ifndef DIGISTRING_STARTUP_TIMER_H
#define DIGISTRING_STARTUP_TIMER_H


#include <chrono>


class StartupTimer {
    public:
        StartupTimer();
        ~StartupTimer();

        /* All get_*_time() functions return number of milliseconds */
        double get_program_time() const;  // Time since program start
        double get_init_time() const;  // Program init time
        void set_init_time();  // Only called once before main loop


    private:
        std::chrono::steady_clock::time_point start_program;
        double init_time;  // Milliseconds
};

extern StartupTimer startup_timer;


#endif  // DIGISTRING_STARTUP_TIMER_H
