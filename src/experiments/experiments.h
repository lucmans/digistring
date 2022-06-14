#ifndef DIGISTRING_EXPERIMENTS_EXPERIMENTS_H
#define DIGISTRING_EXPERIMENTS_EXPERIMENTS_H


#include <map>
#include <string>
#include <functional>  // std::function


void qifft_errors();
void optimize_qxifft();


const std::map<const std::string, const std::function<void()>> str_to_experiment = {
    {"qifft", qifft_errors},
    {"optimize_xqifft", optimize_qxifft},
};


#endif  // DIGISTRING_EXPERIMENTS_EXPERIMENTS_H
