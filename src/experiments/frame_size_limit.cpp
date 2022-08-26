#include "frame_size_limit.h"

#include "note.h"
#include "estimators/estimation_func/window_func.h"
#include "estimators/estimation_func/norms.h"
#include "estimators/estimation_func/interpolate_peaks.h"
#include "error.h"
#include "quit.h"

#include "config/audio.h"

#include <fftw3.h>

#include <algorithm>
#include <vector>
#include <utility>
#include <functional>
#include <sstream>


const int REPS_PER_FREQ = 100;


FrameSizeLimit::FrameSizeLimit(const int _frame_size, const int _padding_size/*, const window_func*/)
        : frame_size(_frame_size), padding_size(_padding_size), in_size(_frame_size + _padding_size) {
    in = (float*)fftwf_malloc(in_size * sizeof(float));
    if(in == NULL) {
        error("Failed to allocate input buffer");
        exit(EXIT_FAILURE);
    }
    // Zero zero-padded part of buffer
    std::fill_n(in + frame_size, padding_size, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles

    out = (fftwf_complex*)fftwf_malloc(((in_size / 2) + 1) * sizeof(fftwf_complex));
    if(out == NULL) {
        error("Failed to allocate Fourier output buffer");
        exit(EXIT_FAILURE);
    }

    // TODO: Exhaustive planner
    p = fftwf_plan_dft_r2c_1d(in_size, in, out, FFTW_ESTIMATE);
    if(p == NULL) {
        error("Failed to create FFTW3 plan");
        exit(EXIT_FAILURE);
    }

    try {
        window_func = new float[frame_size];
    }
    catch(const std::bad_alloc &e) {
        error("Failed to allocate window function buffer");
        exit(EXIT_FAILURE);
    }
    hann_window(window_func, frame_size);
    // blackman_nuttall_window(window_func, frame_size);
    // hann_window(window_func, frame_size);
    // if(!dolph_chebyshev_window(window_func, frame_size, 50.0, true)) {
    //     error("Failed to generate Dolph Chebyshev window");
    //     exit(EXIT_FAILURE);
    // }

    try {
        norms = new double[in_size];
    }
    catch(const std::bad_alloc &e) {
        error("Failed to allocate norms buffer");
        exit(EXIT_FAILURE);
    }
}

FrameSizeLimit::~FrameSizeLimit() {
    delete[] norms;
    delete[] window_func;
    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in);
}


void FrameSizeLimit::test() {
    Note E2(Notes::E, 2);
    Note F2(Notes::F, 2);

    double last_phase = 0.0;
    for(int r = 0; r < REPS_PER_FREQ; r++) {
        const double phase_offset = last_phase * ((double)SAMPLE_RATE / E2.freq);
        for(int i = 0; i < in_size; i++)
            in[i] = sinf((2.0 * M_PI * ((double)i + phase_offset) * E2.freq) / (double)SAMPLE_RATE);
        last_phase = fmod(last_phase + (E2.freq / ((double)SAMPLE_RATE / (double)in_size)), 1.0);

        for(int i = 0; i < frame_size; i++)
            in[i] *= window_func[i];

        fftwf_execute(p);

        calc_norms(out, norms, (in_size / 2) + 1);

        int peak_idx = 1;
        for(int i = 2; i < (in_size / 2); i++)
            if(norms[i] > norms[peak_idx])
                peak_idx = i;

        double amp;
        const double offset = interpolate_max_log(norms[peak_idx], norms[peak_idx - 1], norms[peak_idx + 1], amp);

        const double detected_freq = (peak_idx + offset) * ((double)SAMPLE_RATE / (double)in_size);
        std::cout << Note(detected_freq) << "  (" << detected_freq << ")" << std::endl;
    }
    std::cout << std::endl;

    last_phase = 0.0;
    for(int r = 0; r < REPS_PER_FREQ; r++) {
        const double phase_offset = last_phase * ((double)SAMPLE_RATE / F2.freq);
        for(int i = 0; i < in_size; i++)
            in[i] = sinf((2.0 * M_PI * ((double)i + phase_offset) * F2.freq) / (double)SAMPLE_RATE);
        last_phase = fmod(last_phase + (F2.freq / ((double)SAMPLE_RATE / (double)in_size)), 1.0);

        for(int i = 0; i < frame_size; i++)
            in[i] *= window_func[i];

        fftwf_execute(p);

        calc_norms(out, norms, (in_size / 2) + 1);

        int peak_idx = 1;
        for(int i = 2; i < (in_size / 2); i++)
            if(norms[i] > norms[peak_idx])
                peak_idx = i;

        double amp;
        const double offset = interpolate_max_log(norms[peak_idx], norms[peak_idx - 1], norms[peak_idx + 1], amp);

        const double detected_freq = (peak_idx + offset) * ((double)SAMPLE_RATE / (double)in_size);
        std::cout << Note(detected_freq) << "  (" << detected_freq << ")" << std::endl;
    }
}
