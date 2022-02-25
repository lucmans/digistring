#ifndef DIGISTRING_PERFORMANCE_H
#define DIGISTRING_PERFORMANCE_H


#include <utility>  // std::pair
#include <string>
#include <vector>
#include <chrono>
#include <ostream>


typedef std::pair<std::string, std::chrono::steady_clock::time_point> Timestamp;


const int DEFAULT_DATA_SIZE = 128;  // Largest a ring buffer can be
const int N_FRAMES_FPS = 30;  // Number of frames the average FPS is calculated over


class Performance {
    public:
        Performance();
        ~Performance();

        /* All get_time() functions return number of milliseconds */
        double get_program_time() const;  // Time since program start
        double get_init_time() const;  // Program init time
        void set_init_time();  // Only called once before main loop

        /* Automatic timing and printing functions */
        void push_time_point(const std::string &name);  // Pushes timepoint with given name
        void push_time_point(const std::string &name, const std::chrono::steady_clock::time_point &tp);
        void clear_time_points();  // Call after every frame

        // std::vector<Timestamp>::const_iterator begin() const;
        // std::vector<Timestamp>::const_iterator end() const;
        const std::vector<Timestamp> *get_time_points() const;


    private:
        // Timing
        std::vector<Timestamp> time_points;
        std::chrono::steady_clock::time_point start_program;

        double init_time;  // Milliseconds
};
extern Performance perf;


std::ostream& operator<<(std::ostream &s, const Performance &p);


#endif  // DIGISTRING_PERFORMANCE_H
