#include "interpolate_peaks.h"

#include "note.h"
#include "error.h"

#include <vector>
#include <cmath>


double interpolate_max(const double peak, const double l_neighbor, const double r_neighbor) {
    const double a = l_neighbor,
                 b = peak,
                 c = r_neighbor;
    const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

    return p;
}

double interpolate_max(const double peak, const double l_neighbor, const double r_neighbor, double &amp) {
    const double a = l_neighbor,
                 b = peak,
                 c = r_neighbor;
    const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

    amp = b - (0.25 * (a - c) * p);

    return p;
}

double interpolate_max_log(const double peak, const double l_neighbor, const double r_neighbor) {
    const double a = log(l_neighbor),
                 b = log(peak),
                 c = log(r_neighbor);
    const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

    return p;
}

double interpolate_max_log(const double peak, const double l_neighbor, const double r_neighbor, double &amp) {
    const double a = log(l_neighbor),
                 b = log(peak),
                 c = log(r_neighbor);
    const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

    amp = exp(b - (0.25 * (a - c) * p));

    return p;
}

// dB
double interpolate_max_db(const double peak, const double l_neighbor, const double r_neighbor) {
    const double a = 20.0 * log10(l_neighbor),
                 b = 20.0 * log10(peak),
                 c = 20.0 * log10(r_neighbor);
    const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

    return p;
}

double interpolate_max_db(const double peak, const double l_neighbor, const double r_neighbor, double &amp) {
    const double a = 20.0 * log10(l_neighbor),
                 b = 20.0 * log10(peak),
                 c = 20.0 * log10(r_neighbor);
    const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

    amp = exp10((b - (0.25 * (a - c) * p)) / 20.0);

    return p;
}
