
#include "performance.h"

#include "config.h"
#include "error.h"

#include <string>
#include <vector>
#include <ostream>


Performance perf;


Performance::Performance() {
    start_program = std::chrono::steady_clock::now();
    init_time = -1;
}

Performance::~Performance() {

}


double Performance::get_program_time() const {
    std::chrono::duration<double, std::milli> dur = std::chrono::steady_clock::now() - start_program;

    return dur.count();
}


double Performance::get_init_time() const {
    if(init_time < 0) {
        warning("Tried to get init time before setting init time");
        return -1;
    }

    return init_time;
}

void Performance::set_init_time() {
    // Only allow being called once
    static bool called = false;
    if(called) {
        warning("Performance::set_init_time() is called again");
        return;
    }
    called = true;

    std::chrono::duration<double, std::milli> t = std::chrono::steady_clock::now() - start_program;
    init_time = t.count();
}


void Performance::push_time_point(const std::string &name) {
    time_points.push_back({name, std::chrono::steady_clock::now()});
}

void Performance::push_time_point(const std::string &name, const std::chrono::steady_clock::time_point &tp) {
    time_points.push_back({name, tp});
}


void Performance::clear_time_points() {
    time_points.clear();
}


// std::vector<Timestamp>::const_iterator Performance::begin() const {
//     return time_points.cbegin();
// }

// std::vector<Timestamp>::const_iterator Performance::end() const {
//     return time_points.cend();
// }

const std::vector<Timestamp> *Performance::get_time_points() const {
    return &time_points;
}


std::ostream& operator<<(std::ostream &s, const Performance &p) {
    const std::vector<Timestamp> *time_points = p.get_time_points();
    std::chrono::duration<double, std::milli> dur = (*time_points)[time_points->size() - 1].second - (*time_points)[0].second;
    const double frame_time = dur.count();

    s.precision(3);
    s << "Frame time usage: " << frame_time << " ms" << std::endl;
    for(size_t i = 1; i < time_points->size(); i++) {
        dur = (*time_points)[i].second - (*time_points)[i - 1].second;
        s << "  " << (*time_points)[i].first << ": " << dur.count() << " ms  (" << (dur.count() / frame_time) * 100.0 << "%)" << std::endl;
    }

    return s;
}
