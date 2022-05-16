#include "basic_fourier.h"

#include "error.h"
#include "performance.h"
#include "note.h"

#include "window_func.h"
#include "estimation_func.h"

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
    if constexpr(!HEADLESS)
        delete estimator_graphics;

    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in);
}


Estimators BasicFourier::get_type() const {
    return Estimators::basic_fourier;
}


void BasicFourier::all_max(const double norms[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks) {
    for(int i = 1; i < (FRAME_SIZE / 2); i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1])
            peaks.push_back(i);
    }
}

void BasicFourier::all_max(const double norms[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks, double &max_norm) {
    for(int i = 1; i < (FRAME_SIZE / 2); i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1]) {
            peaks.push_back(i);
            if(norms[i] > max_norm)
                max_norm = norms[i];
        }
    }
}


void BasicFourier::get_loudest_peak(NoteSet &out_notes, const double norms[(FRAME_SIZE / 2) + 1], const std::vector<int> &peaks) {
    const int n_peaks = peaks.size();
    if(n_peaks == 0)
        return;
    else if(n_peaks == 1) {
        const Note tuned = Note(peaks[0] * ((double)SAMPLE_RATE / (double)FRAME_SIZE));
        out_notes.push_back(Note(tuned.midi_number, norms[peaks[0]]));
        return;
    }

    int max_idx = 0;
    for(int i = 1; i < n_peaks; i++) {
        if(norms[peaks[i]] > norms[peaks[max_idx]])
            max_idx = i;
    }

    const Note tuned = Note(peaks[max_idx] * ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    out_notes.push_back(Note(tuned.midi_number, norms[peaks[max_idx]]));
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
    // perf.push_time_point("Applied window function");

    // Do the actual transform
    fftwf_execute(p);
    // perf.push_time_point("Fourier transformed");

    // Calculate amplitude of every frequency component
    double norms[(FRAME_SIZE / 2) + 1];
    double power, max_norm;
    calc_norms(out, norms, (FRAME_SIZE / 2) + 1, max_norm, power);
    // perf.push_time_point("Norms calculated");

    // Find peaks based on envelope
    std::vector<int> peaks;
    all_max(norms, peaks, max_norm);

    /* Note estimation from peaks */
    NoteSet noteset;  // Noteset, so polyphony can easily be supported
    get_loudest_peak(noteset, norms, peaks);

    // Add note to output note_events if not filtered
    if(noteset.size() > 0)
        note_events.push_back(NoteEvent(noteset[0], FRAME_SIZE, 0));


    // Graphics
    if constexpr(!HEADLESS) {
        BasicFourierGraphics *const basic_fourier_graphics = static_cast<BasicFourierGraphics *>(estimator_graphics);
        basic_fourier_graphics->set_last_max_recorded_value(max_norm);

        Spectrum &spectrum = basic_fourier_graphics->get_spectrum();
        spectrum.clear();

        // Start at i = 1 to skip rendering DC offset (envelope has no DC offset, so do first explicitly)
        for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++)
            spectrum.add_data(i * ((double)SAMPLE_RATE / (double)FRAME_SIZE), norms[i], (double)SAMPLE_RATE / (double)FRAME_SIZE);
        spectrum.sort();

        std::vector<double> &f_peaks = basic_fourier_graphics->get_peaks();
        f_peaks.clear();
        for(int peak_bin : peaks)
            f_peaks.push_back(peak_bin * ((double)SAMPLE_RATE / (double)FRAME_SIZE));
        std::sort(f_peaks.begin(), f_peaks.end());
    }
}
