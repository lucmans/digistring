
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


/* static */ void Estimator::free_input_buffer(float *const input_buffer) {
    fftwf_free(input_buffer);
}


double Estimator::get_max_norm() const {
    return max_norm;
}

#include "../config.h"
void Estimator::get_data_point(const double *&out_norms, int &norms_size) const {
    out_norms = norms;
    norms_size = (FRAME_SIZE / 2) + 1;
}
