
#include "estimator.h"

#include "spectrum.h"
#include "error.h"

#include "config/graphics.h"

#include <fftw3.h>

#include <map>
#include <string>
#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE


Estimator::Estimator() {
    max_norm = 0.0;
}

Estimator::~Estimator() {

}


double Estimator::get_max_norm() const {
    if constexpr(HEADLESS) {
        error("This call should never occur with headless mode");
        exit(EXIT_FAILURE);
    }

    return max_norm;
}

const Spectrum *Estimator::get_spectrum() {
    if constexpr(HEADLESS) {
        error("This call should never occur with headless mode");
        exit(EXIT_FAILURE);
    }

    spectrum.sort();
    return &spectrum;
}
