
#include "highres.h"

#include "../config.h"
#include "../error.h"
#include "../performance.h"

#include "window_func.h"
#include "estimation_func.h"

#include <fftw3.h>


HighRes::HighRes(float *const input_buffer) {
    // Input buffer is allocated by caller, as it is shared between multiple objects

    out = (fftwf_complex*)fftwf_malloc(((FRAME_SIZE / 2) + 1) * sizeof(fftwf_complex));
    if(out == NULL) {
        error("Failed to malloc output buffer");
        exit(EXIT_FAILURE);
    }

    p = fftwf_plan_dft_r2c_1d(FRAME_SIZE, input_buffer, out, FFTW_ESTIMATE);

    // Pre-calculate window function
    blackman_nuttall_window(window_func);
}

HighRes::~HighRes() {
    fftwf_destroy_plan(p);
    // fftwf_cleanup();
    fftwf_free(out);
}


Estimators HighRes::get_type() const {
    return Estimators::highres;
}


/*static*/ float *HighRes::create_input_buffer(int &buffer_size) {
    float *input_buffer = (float*)fftwf_malloc(FRAME_SIZE * sizeof(float));
    if(input_buffer == NULL) {
        error("Failed to malloc input buffer");
        exit(EXIT_FAILURE);
    }

    buffer_size = FRAME_SIZE;
    return input_buffer;
}

float *HighRes::_create_input_buffer(int &buffer_size) const {
    return HighRes::create_input_buffer(buffer_size);
}

// Implemented by superclass
// void HighRes::free_input_buffer(float *const input_buffer) const {
//     fftwf_free(input_buffer);
// }


void HighRes::perform(float *const input_buffer) {
    // Apply window function to minimize spectral leakage
    for(int i = 0; i < FRAME_SIZE; i++)
        input_buffer[i] *= (float)window_func[i];  // TODO: Have float versions of the window functions
    perf.push_time_point("Applied window function");

    // Do the actual transform
    fftwf_execute(p);
    perf.push_time_point("Fourier transformed");

    double norms[(FRAME_SIZE / 2) + 1] = {};
    double power;
    calc_norms(out, norms, (FRAME_SIZE / 2) + 1, max_norm, power);
    perf.push_time_point("Norms calculated");

    if constexpr(!HEADLESS) {
        spectrum.clear();
        for(int i = 0; i < (FRAME_SIZE / 2) + 1; i++)
            spectrum.add_data(i * ((double)SAMPLE_RATE / (double)FRAME_SIZE), norms[i], (double)SAMPLE_RATE / (double)FRAME_SIZE);
    }
}
