#include "performance.h"

#include "error.h"
#include "config/cli_args.h"

#include <string>
#include <vector>
#include <ostream>

#include <map>
#include <fstream>


Performance perf;


Performance::Performance() {
    start_program = std::chrono::steady_clock::now();
    init_time = -1;
}

Performance::~Performance() {
    if(cli_args.perf_output_file == "")
        return;

    const std::string filename = cli_args.perf_output_file;
    info("Writing Digistring's performance statistics to '" + filename + "'");

    std::fstream output_stream(filename, std::fstream::out);
    if(!output_stream.is_open()) {
        error("Failed to create/open file '" + filename + "'");
        exit(EXIT_FAILURE);
    }

    for(const auto &[desc, times] : durations) {
        output_stream << desc << std::endl;  // << "  (" << times.size() << " timepoints)" << std::endl;
        for(const auto &t : times)
            output_stream << t << ' ';
        output_stream << std::endl << std::endl;
    }

    output_stream.close();
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
    // Only save durations if outputting it to a file
    if(cli_args.perf_output_file != "") {
        const size_t n_time_points = time_points.size();
        if(n_time_points < 2) {
            time_points.clear();
            return;
        }

        const std::string total_str = "Total frame time";
        const std::chrono::duration<double, std::milli> total_dur = time_points[n_time_points - 1].second - time_points[0].second;
        durations[total_str].push_back(total_dur.count());

        for(size_t i = 1; i < n_time_points; i++) {
            if(time_points[i].first == total_str) {
                error("Timepoint label is the same as total frame time label (" + total_str + ")");
                exit(EXIT_FAILURE);
            }

            const std::chrono::duration<double, std::milli> dur = time_points[i].second - time_points[i - 1].second;
            durations[time_points[i].first].push_back(dur.count());
        }
    }

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
    const std::vector<Timestamp> *const time_points = p.get_time_points();

    const size_t n_time_points = time_points->size();
    if(n_time_points < 2) {
        warning("Need at least 2 time points for any performance statistics");
        return s;
    }

    std::chrono::duration<double, std::milli> dur = (*time_points)[n_time_points - 1].second - (*time_points)[0].second;
    const double frame_time = dur.count();

    s.precision(3);
    s << "Frame time usage: " << frame_time << " ms" << std::endl;
    for(size_t i = 1; i < n_time_points; i++) {
        dur = (*time_points)[i].second - (*time_points)[i - 1].second;
        s << "  " << (*time_points)[i].first << ": " << dur.count() << " ms  (" << (dur.count() / frame_time) * 100.0 << "%)" << std::endl;
    }

    return s;
}
