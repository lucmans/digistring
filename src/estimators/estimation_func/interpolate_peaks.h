#ifndef DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_INTERPOLATE_PEAKS_H
#define DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_INTERPOLATE_PEAKS_H


// #include <vector>

// #include "note.h"


// Normal scale
double interpolate_max(const double peak, const double l_neighbor, const double r_neighbor);
double interpolate_max(const double peak, const double l_neighbor, const double r_neighbor, double &amp);

// Log scale
double interpolate_max_log(const double peak, const double l_neighbor, const double r_neighbor);
double interpolate_max_log(const double peak, const double l_neighbor, const double r_neighbor, double &amp);

// dB scale (20 * log10(peak))
double interpolate_max_db(const double peak, const double l_neighbor, const double r_neighbor);
double interpolate_max_db(const double peak, const double l_neighbor, const double r_neighbor, double &amp);


// void interpolate_peaks(NoteSet &noteset, const double norms[], const int n_norms, const std::vector<int> &peaks);


#endif  // DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_INTERPOLATE_PEAKS_H
