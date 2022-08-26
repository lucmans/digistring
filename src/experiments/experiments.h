#ifndef DIGISTRING_EXPERIMENTS_EXPERIMENTS_H
#define DIGISTRING_EXPERIMENTS_EXPERIMENTS_H


#include <map>
#include <string>
#include <functional>  // std::function


void qifft_errors();
void optimize_qxifft();
void frame_size_limit();


const std::map<const std::string, const std::function<void()>> str_to_experiment = {
    {"qifft", qifft_errors},
    {"optimize_xqifft", optimize_qxifft},
    {"frame_size_limit", frame_size_limit},
};


#endif  // DIGISTRING_EXPERIMENTS_EXPERIMENTS_H
