#ifndef DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_PEAK_PICKERS_H
#define DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_PEAK_PICKERS_H


#include <vector>


/* TODO: Description */
void all_max(const double norms[], const int n_norms, std::vector<int> &peaks);

// With signal to noise filter
void all_max(const double norms[], const int n_norms, std::vector<int> &peaks, const int max_norm);


/* TODO: Description */
void envelope_peaks(const double norms[], const double envelope[], const int n_norms, std::vector<int> &peaks);

// With signal to noise filter
void envelope_peaks(const double norms[], const double envelope[], const int n_norms, std::vector<int> &peaks, const int max_norm);


/* TODO: Description */
void min_dy_peaks(const double norms[], const int n_norms, std::vector<int> &peaks);


#endif  // DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_PEAK_PICKERS_H
