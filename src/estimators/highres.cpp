#include "highres.h"

#include "error.h"
#include "note.h"

#include "estimation_func/window_func.h"
#include "estimation_func/norms.h"
#include "estimation_func/envelope.h"
#include "estimation_func/peak_pickers.h"
#include "estimation_func/interpolate_peaks.h"
#include "estimation_func/note_selectors.h"

#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"

#include <fftw3.h>

#include <cmath>
#include <algorithm>
#include <vector>


HighRes::HighRes(float *&input_buffer, int &buffer_size) : perf("HighRes") {
    // Let the called know the number of samples to request from SampleGetter each call
    buffer_size = FRAME_SIZE;

    // fftw3 input buffer to Fourier on
    in = (float*)fftwf_malloc(FRAME_SIZE_PADDED * sizeof(float));
    if(in == NULL) {
        error("Failed to malloc sample input buffer");
        exit(EXIT_FAILURE);
    }
    input_buffer = in;  // Share the input buffer with caller, so SampleGetter can directly write samples to it (no copies needed)

    // Zero zero-padded part of buffer
    std::fill_n(in + FRAME_SIZE, FRAME_SIZE_PADDED - FRAME_SIZE, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles

    out = (fftwf_complex*)fftwf_malloc(((FRAME_SIZE_PADDED / 2) + 1) * sizeof(fftwf_complex));
    if(out == NULL) {
        error("Failed to malloc Fourier output buffer");
        exit(EXIT_FAILURE);
    }

    // TODO: Exhaustive planner and graphics to show "optimizing planner"
    p = fftwf_plan_dft_r2c_1d(FRAME_SIZE_PADDED, input_buffer, out, FFTW_ESTIMATE);
    if(p == NULL) {
        error("Failed to create FFTW3 plan");
        exit(EXIT_FAILURE);
    }

    // Pre-calculate window function
    if(!dolph_chebyshev_window(window_func, FRAME_SIZE, DEFAULT_ATTENUATION, true)) {
        // dolph_chebyshev_window() already prints error
        warning("Failed to get Dolph Chebyshev window; using Blackman Nuttall window instead...");

        blackman_nuttall_window(window_func, FRAME_SIZE);
    }

    // Pre-calculate Gaussian for envelope computation
    gaussian_envelope(NULL, NULL, 0);

    if constexpr(!HEADLESS) {
        HighResGraphics *const tmp_graphics = new HighResGraphics();
        estimator_graphics = tmp_graphics;

        std::vector<float> &tmp_wave_samples = tmp_graphics->get_wave_samples();
        tmp_wave_samples.clear();  // Just to be sure
        tmp_wave_samples.resize(FRAME_SIZE, 0.0);
    }

    prev_power = 0.0;
}

HighRes::~HighRes() {
    if constexpr(!HEADLESS)
        delete estimator_graphics;

    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in);
}


Estimators HighRes::get_type() const {
    return Estimators::highres;
}


void HighRes::interpolate_peaks(NoteSet &noteset, const double norms[(FRAME_SIZE_PADDED / 2) + 1], const std::vector<int> &peaks) {
    for(int peak : peaks) {
        // Check if the interpolation will be in-bounds
        if(peak == 0 || peak == FRAME_SIZE_PADDED / 2) {
            warning("Peak on first or last bin");
            continue;
        }
        else if(peak > FRAME_SIZE_PADDED / 2) {
            error("Peak found outside bins");
            exit(EXIT_FAILURE);
        }

        double amp;
        const double offset = interpolate_max_log(norms[peak], norms[peak - 1], norms[peak + 1], amp);
        const double freq = ((double)SAMPLE_RATE / (double)FRAME_SIZE_PADDED) * (peak + offset);
        noteset.push_back(Note(freq, amp));
    }
}


void HighRes::perform(float *const input_buffer, NoteEvents &note_events) {
    // Safe raw waveform before applying window function
    if constexpr(!HEADLESS) {
        HighResGraphics *const highres_graphics = static_cast<HighResGraphics *>(estimator_graphics);

        std::vector<float> &wave_samples = highres_graphics->get_wave_samples();
        memcpy(wave_samples.data(), input_buffer, FRAME_SIZE * sizeof(float));
    }

    perf.clear_time_points();
    perf.push_time_point("Start");

    /* Fourier transform */
    // Apply window function to minimize spectral leakage
    for(int i = 0; i < FRAME_SIZE; i++)
        input_buffer[i] *= window_func[i];
    perf.push_time_point("Applied window function");

    // Do the actual transform
    fftwf_execute(p);
    perf.push_time_point("Executed FFT");

    // Calculate amplitude of every frequency component
    double norms[(FRAME_SIZE_PADDED / 2) + 1];
    double power, max_norm;
    calc_norms(out, norms, (FRAME_SIZE_PADDED / 2) + 1, max_norm, power);
    perf.push_time_point("Calculated norms");

    /* Peak picking */
    // Compute Gaussian envelope
    double envelope[(FRAME_SIZE_PADDED / 2) + 1];
    gaussian_envelope(norms, envelope, (FRAME_SIZE_PADDED / 2) + 1);
    perf.push_time_point("Calculated Gaussian envelope");

    // TODO: Convex envelope
    // Start from highest/lowest peaks, then advance both left/right to next highest peak until no peaks left

    // Find peaks based on envelope
    std::vector<int> peaks;
    if(power > POWER_THRESHOLD)
        envelope_peaks(norms, envelope, (FRAME_SIZE_PADDED / 2) + 1, peaks, max_norm);
        // all_max(norms, (FRAME_SIZE_PADDED / 2) + 1, peaks, max_norm);
        // all_max(norms, (FRAME_SIZE_PADDED / 2) + 1, peaks);
    perf.push_time_point("Selected peaks");

    // // Find peaks on min-dy
    // std::vector<int> peaks;
    // min_dy_peaks(norms, peaks);

    /* Note estimation from peaks */
    // Interpolate peak locations
    NoteSet i_peaks;
    interpolate_peaks(i_peaks, norms, peaks);
    perf.push_time_point("Interpolated peaks");

    // Extract played note from the peaks
    NoteSet noteset;  // Noteset, so polyphony can easily be supported
    // get_loudest_peak(noteset, i_peaks);
    // get_lowest_peak(noteset, i_peaks);
    get_most_overtones(noteset, i_peaks);
    // get_most_overtone_power(noteset, i_peaks);
    perf.push_time_point("Created noteset");

    // Add note to output note_events if not filtered
    if(noteset.size() > 0) {
        bool add_note = true;
        if constexpr(LOW_HIGH_FILTER) {
            if(noteset[0].midi_number < LOWEST_NOTE.midi_number || noteset[0].midi_number > HIGHEST_NOTE.midi_number)
                add_note = false;
        }

        if constexpr(TRANSIENT_FILTER) {
            if(power > prev_power + TRANSIENT_FILTER_POWER)
                add_note = false;
        }

        if(add_note)
            note_events.push_back(NoteEvent(noteset[0], FRAME_SIZE, 0));
    }
    prev_power = power;
    perf.push_time_point("Filtered notes");


    // Graphics
    if constexpr(!HEADLESS) {
        HighResGraphics *const highres_graphics = static_cast<HighResGraphics *>(estimator_graphics);
        highres_graphics->set_last_max_recorded_value(max_norm);

        Spectrum &spectrum = highres_graphics->get_spectrum();
        spectrum.clear();

        Spectrum &envelope_spectrum = highres_graphics->get_envelope();
        envelope_spectrum.clear();

        // Start at i = 1 to skip rendering DC offset (envelope has no DC offset, so do first explicitly)
        envelope_spectrum.add_data(0.0, envelope[0], 0.0);
        for(int i = 1; i < (FRAME_SIZE_PADDED / 2) + 1; i++) {
            spectrum.add_data(i * ((double)SAMPLE_RATE / (double)FRAME_SIZE_PADDED), norms[i], (double)SAMPLE_RATE / (double)FRAME_SIZE_PADDED);
            envelope_spectrum.add_data(i * ((double)SAMPLE_RATE / (double)FRAME_SIZE_PADDED), envelope[i], 0.0);
        }
        spectrum.sort();
        envelope_spectrum.sort();


        std::vector<double> &f_peaks = highres_graphics->get_peaks();
        f_peaks.clear();
        for(int peak_bin : peaks)
            f_peaks.push_back(peak_bin * ((double)SAMPLE_RATE / (double)FRAME_SIZE_PADDED));
        std::sort(f_peaks.begin(), f_peaks.end());
    }
}
