#include "performance.h"

#include "error.h"
#include "config/cli_args.h"

#include <string>
#include <vector>
#include <ostream>
#include <filesystem>  // std::filesystem::exists()

#include <map>
#include <fstream>


Performance::Performance(const std::string _subtask) {
    // Don't find a filename if no output file is going to be created
    if(cli_args.perf_output_file == "")
        return;

    subtask = _subtask;

    // Original filename
    const std::string o_filename = cli_args.perf_output_file;

    // Separate basename and extension
    std::string o_basename, o_extension;
    const size_t pos = o_filename.find_last_of('.');
    if(pos == std::string::npos || pos == 0)
        o_basename = o_filename;
    else {  // pos > 0
        o_basename = o_filename.substr(0, pos);
        o_extension = o_filename.substr(pos);
    }
    const std::string o_ext_filename = o_basename + (_subtask != "" ? '_' + _subtask : "") + o_extension;

    // Try to generate a new unique filename inserting a number between name and extension
    std::string g_filename = o_ext_filename;  // Generated filename
    for(int i = 2; std::filesystem::exists(g_filename) || Performance::outfiles.count(g_filename) == 1; i++)
        g_filename = o_basename + (_subtask != "" ? '_' + _subtask : "") + '_' + std::to_string(i) + o_extension;

    if(g_filename != o_ext_filename)
        warning("File '" + o_ext_filename + "' already exists; naming it '" + g_filename + "' instead");

    outfile = g_filename;
    Performance::outfiles.insert(g_filename);
}

Performance::~Performance() {
    if(cli_args.perf_output_file == "")
        return;

    if(durations.size() == 0) {
        warning((subtask == "" ? "main" : subtask) + " performance counter has no timepoints");
        return;
    }

    info("Writing Digistring's " + (subtask == "" ? "main" : subtask) + " performance statistics to '" + outfile + "'");

    std::fstream output_stream(outfile, std::fstream::out);
    if(!output_stream.is_open()) {
        error("Failed to create/open file '" + outfile + "'");
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

        const std::string total_str = "Total time";
        const std::chrono::duration<double, std::milli> total_dur = time_points[n_time_points - 1].second - time_points[0].second;
        durations[total_str].push_back(total_dur.count());

        for(size_t i = 1; i < n_time_points; i++) {
            if(time_points[i].first == total_str) {
                error("Timepoint label is the same as total time label (" + total_str + ")");
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
