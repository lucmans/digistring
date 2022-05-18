#include "basic_fourier.h"

#include "error.h"
#include "performance.h"
#include "note.h"

#include "window_func.h"

#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"

#include <fftw3.h>

#include <cmath>
#include <algorithm>
#include <vector>


BasicFourier::BasicFourier(float *&input_buffer, int &buffer_size) {
    // Let the called know the number of samples to request from SampleGetter each call
    buffer_size = FRAME_SIZE;

    // fftw3 input buffer to Fourier on
    in = (float*)fftwf_malloc(FRAME_SIZE * sizeof(float));
    if(in == NULL) {
        error("Failed to malloc sample input buffer");
        exit(EXIT_FAILURE);
    }
    input_buffer = in;  // Share the input buffer with caller, so SampleGetter can directly write samples to it (no copies needed)

    out = (fftwf_complex*)fftwf_malloc(((FRAME_SIZE / 2) + 1) * sizeof(fftwf_complex));
    if(out == NULL) {
        error("Failed to malloc Fourier output buffer");
        exit(EXIT_FAILURE);
    }

    // TODO: Exhaustive planner and graphics to show "optimizing planner"
    p = fftwf_plan_dft_r2c_1d(FRAME_SIZE, input_buffer, out, FFTW_ESTIMATE);

    // Pre-calculate window function
    if(!dolph_chebyshev_window(window_func, FRAME_SIZE, DEFAULT_ATTENUATION, true)) {
        // dolph_chebyshev_window() already prints error
        warning("Failed to get Dolph Chebyshev window; using Blackman Nuttall window instead...");

        blackman_nuttall_window(window_func, FRAME_SIZE);
    }

    if constexpr(!HEADLESS) {
        try {
            norms = new double[(FRAME_SIZE / 2) + 1];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to allocate norms buffer (" + STR(e.what()) + ")");
            hint("Try using a smaller frame size or enable headless mode");
            exit(EXIT_FAILURE);
        }

        BasicFourierGraphics *const tmp_graphics = new BasicFourierGraphics();
        estimator_graphics = tmp_graphics;

        // No envelope for Basic Fourier pitch estimator
        Spectrum &envelope_spectrum = tmp_graphics->get_envelope();
        envelope_spectrum.clear();

        std::vector<float> &tmp_wave_samples = tmp_graphics->get_wave_samples();
        tmp_wave_samples.clear();  // Just to be sure
        tmp_wave_samples.resize(FRAME_SIZE, 0.0);
    }
}

BasicFourier::~BasicFourier() {
    if constexpr(!HEADLESS) {
        delete estimator_graphics;
        delete[] norms;
    }

    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in);
}


Estimators BasicFourier::get_type() const {
    return Estimators::basic_fourier;
}


void BasicFourier::perform(float *const input_buffer, NoteEvents &note_events) {
    // Safe raw waveform before applying window function
    if constexpr(!HEADLESS) {
        BasicFourierGraphics *const basic_fourier_graphics = static_cast<BasicFourierGraphics *>(estimator_graphics);

        std::vector<float> &wave_samples = basic_fourier_graphics->get_wave_samples();
        memcpy(wave_samples.data(), input_buffer, FRAME_SIZE * sizeof(float));
    }

    /* Fourier transform */
    // Apply window function to minimize spectral leakage
    for(int i = 0; i < FRAME_SIZE; i++)
        input_buffer[i] *= window_func[i];

    // Do the actual transform
    fftwf_execute(p);

    // Get the bin with maximum signal power
    double max_norm_val = -1.0;
    int max_norm_idx = -1;
    for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++) {
        const double tmp_norm = sqrt((out[i][0] * out[i][0]) + (out[i][1] * out[i][1]));

        if(tmp_norm > max_norm_val) {
            max_norm_val = tmp_norm;
            max_norm_idx = i;
        }

        if constexpr(!HEADLESS)
            norms[i] = tmp_norm;
    }

    // Calculate DC offset explicitly
    if constexpr(!HEADLESS)
        norms[0] = sqrt((out[0][0] * out[0][0]) + (out[0][1] * out[0][1]));

    // Add note to output note_events
    if(max_norm_idx != -1)
        note_events.push_back(NoteEvent(Note(max_norm_idx * ((double)SAMPLE_RATE / (double)FRAME_SIZE), max_norm_val), FRAME_SIZE, 0));


    // Graphics
    if constexpr(!HEADLESS) {
        BasicFourierGraphics *const basic_fourier_graphics = static_cast<BasicFourierGraphics *>(estimator_graphics);
        basic_fourier_graphics->set_last_max_recorded_value(max_norm_val);

        Spectrum &spectrum = basic_fourier_graphics->get_spectrum();
        spectrum.clear();

        // Start at i = 1 to skip rendering DC offset (envelope has no DC offset, so do first explicitly)
        for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++)
            spectrum.add_data(i * ((double)SAMPLE_RATE / (double)FRAME_SIZE), norms[i], (double)SAMPLE_RATE / (double)FRAME_SIZE);
        spectrum.sort();

        std::vector<double> &f_peaks = basic_fourier_graphics->get_peaks();
        f_peaks.clear();
        f_peaks.push_back(max_norm_idx * ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    }
}
