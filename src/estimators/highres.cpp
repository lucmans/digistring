
#include "highres.h"

#include "../config.h"
#include "../error.h"
#include "../performance.h"

#include "window_func.h"

#include <fftw3.h>


HighRes::HighRes() {
    in = (float*)fftwf_malloc(FRAME_SIZE * sizeof(float));
    if(in == NULL) {
        error("Failed to malloc input buffer");
        exit(EXIT_FAILURE);
    }

    out = (fftwf_complex*)fftwf_malloc(((FRAME_SIZE / 2) + 1) * sizeof(fftwf_complex));
    if(out == NULL) {
        error("Failed to malloc output buffer");
        exit(EXIT_FAILURE);
    }

    p = fftwf_plan_dft_r2c_1d(FRAME_SIZE, in, out, FFTW_ESTIMATE);

    // Pre-calculate window function
    blackman_nuttall_window(window_func);
}

HighRes::~HighRes() {
    fftwf_free(out);
    fftwf_destroy_plan(p);
}


Estimators HighRes::get_type() const {
    return Estimators::highres;
}


float *HighRes::get_input_buffer() const {
    return in;
}

float *HighRes::get_input_buffer(int &buffer_size) const {
    buffer_size = FRAME_SIZE;

    return in;
}


void HighRes::perform() {
    // Apply window function to minimize spectral leakage
    for(int i = 0; i < FRAME_SIZE; i++)
        in[i] *= (float)window_func[i];  // TODO: Have float versions of the window functions
    perf.push_time_point("Applied window function");

    // Do the actual transform
    fftwf_execute(p);
    perf.push_time_point("Fourier transformed");

    // double norms[(FRAME_SIZE / 2) + 1] = {};
    // double power, max_norm;
    // calc_norms(out, norms, max_norm, power);
    // perf.push_time_point("Norms calculated");

    // if(!settings.headless) {
    //     graphics->set_max_recorded_value_if_larger(max_norm);
    //     graphics->add_data_point(norms);
    // }
}
