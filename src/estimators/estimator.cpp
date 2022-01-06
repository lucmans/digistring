
#include "estimator.h"

#include "../config.h"
#include "../error.h"

#include "../spectrum.h"

#include <fftw3.h>

#include <map>
#include <string>


const std::map<const Estimators, const std::string> EstimatorString = {{Estimators::highres, "highres"},
                                                                       {Estimators::tuned, "tuned"}};


Estimator::Estimator() {
    max_norm = 0.0;
}

Estimator::~Estimator() {

}


/* static */ void Estimator::free_input_buffer(float *const input_buffer) {
    fftwf_free(input_buffer);
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
