
#include "estimator.h"

#include <fftw3.h>

#include <map>
#include <string>


const std::map<const Estimators, const std::string> EstimatorString = {{Estimators::highres, "highres"},
                                                                       {Estimators::tuned, "tuned"}};


Estimator::Estimator() {

}

Estimator::~Estimator() {

}


void Estimator::free_input_buffer(float *const input_buffer) const {
    fftwf_free(input_buffer);
}
