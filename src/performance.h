#ifndef DIGISTRING_PERFORMANCE_H
#define DIGISTRING_PERFORMANCE_H


#include <utility>  // std::pair
#include <string>
#include <vector>
#include <chrono>
#include <ostream>

#include <map>
#include <set>


typedef std::pair<std::string, std::chrono::steady_clock::time_point> Timestamp;


class Performance {
    public:
        Performance(const std::string subtask);
        Performance() : Performance("") {};
        ~Performance();

        void push_time_point(const std::string &name);  // Pushes timepoint now with given name
        void push_time_point(const std::string &name, const std::chrono::steady_clock::time_point &tp);

        // Writes the time point durations to durations if output file is set in cli_args
        void clear_time_points();

        // For perf object printing (ostream << overload)
        // std::vector<Timestamp>::const_iterator begin() const;
        // std::vector<Timestamp>::const_iterator end() const;
        const std::vector<Timestamp> *get_time_points() const;


    private:
        std::vector<Timestamp> time_points;
        std::map<const std::string, std::vector<double>> durations;

        inline static std::set<std::string> outfiles;
        std::string subtask;
        std::string outfile;
};


std::ostream& operator<<(std::ostream &s, const Performance &p);


#endif  // DIGISTRING_PERFORMANCE_H
