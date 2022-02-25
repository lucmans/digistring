#include "highres.h"

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


// inline double interpolate_max(const int max_idx, const double norms[(FRAME_SIZE / 2) + 1]) {
//     const double a = log2(norms[max_idx - 1]),
//                  b = log2(norms[max_idx]),
//                  c = log2(norms[max_idx + 1]);
//     const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

//     return max_idx + p;
// }

inline double interpolate_max(const int max_idx, const double norms[(FRAME_SIZE / 2) + 1], double &amp) {
    const double a = log2(norms[max_idx - 1]),
                 b = log2(norms[max_idx]),
                 c = log2(norms[max_idx + 1]);
    const double p = 0.5 * ((a - c) / (a - (2.0 * b) + c));

    amp = b - (0.25 * (a - c) * p);

    return max_idx + p;
}


HighRes::HighRes(float *&input_buffer, int &buffer_size) {
    // in_size = FRAME_SIZE;  // Class member buffer size
    // buffer_size = in_size;  // Call by reference parameter to inform caller of buffer size

    buffer_size = FRAME_SIZE;

    input_buffer = (float*)fftwf_malloc(FRAME_SIZE * sizeof(float));
    if(input_buffer == NULL) {
        error("Failed to malloc sample input buffer");
        exit(EXIT_FAILURE);
    }

    in = input_buffer;  // Share the sample input buffer with the Fourier input buffer
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

    // Pre-calculate Gaussian for envelope computation
    for(int i = 0; i < KERNEL_WIDTH; i++)
        gaussian[i] = exp(-M_PI * ((double)(i - MID) / ((double)MID * SIGMA)) * ((double)(i - MID) / ((double)MID * SIGMA)));

    estimator_graphics = new HighResGraphics();
}

HighRes::~HighRes() {
    delete estimator_graphics;

    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in);
}


Estimators HighRes::get_type() const {
    return Estimators::highres;
}


void HighRes::calc_envelope(const double norms[(FRAME_SIZE / 2) + 1], double envelope[(FRAME_SIZE / 2) + 1]) {
    for(int i = 0; i < (FRAME_SIZE / 2) + 1; i++) {
        double sum = 0, weights = 0;
        for(int j = std::max(-MID, -i); j <= std::min(MID, (FRAME_SIZE / 2) - i); j++) {
            sum += norms[i + j] * gaussian[j + MID];
            weights += gaussian[j + MID];
        }
        envelope[i] = sum / weights;
    }
}


void HighRes::all_max(const double norms[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks) {
    for(int i = 1; i < (FRAME_SIZE / 2); i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1])
            peaks.push_back(i);
    }

    // Filter quiet peaks
    for(size_t i = peaks.size(); i > 0; i--) {
        if(norms[peaks[i - 1]] < PEAK_THRESHOLD)
            peaks.erase(peaks.begin() + (i - 1));
    }
}

void HighRes::envelope_peaks(const double norms[(FRAME_SIZE / 2) + 1], const double envelope[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks) {
    for(int i = 5; i < (FRAME_SIZE / 2); i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1]  // A local maximum
           && norms[i] > envelope[i]  // Higher than envelope
           && envelope[i] > 0.1)  // Filter quiet peaks
            peaks.push_back(i);
    }
}


void HighRes::interpolate_peaks(NoteSet &noteset, const double norms[(FRAME_SIZE / 2) + 1], const std::vector<int> &peaks) {
    for(int peak : peaks) {
        // Check if the interpolation will be in-bounds
        if(peak == 0 || peak == FRAME_SIZE / 2) {
            warning("Peak on first or last bin");
            continue;
        }
        else if(peak > FRAME_SIZE / 2) {
            error("Peak found outside bins");
            exit(-1);
        }

        double amp;
        const double freq = ((double)SAMPLE_RATE / FRAME_SIZE) * interpolate_max(peak, norms, amp);
        noteset.push_back(Note(freq, amp));
    }
}


void HighRes::get_loudest_peak(NoteSet &out_notes, const NoteSet &candidate_notes) {
    const int n_notes = candidate_notes.size();
    if(n_notes == 0)
        return;
    else if(n_notes == 1) {
        out_notes.push_back(candidate_notes[0]);
        return;
    }

    int max_idx = 0;
    for(int i = 1; i < n_notes; i++) {
        if(candidate_notes[i].amp > candidate_notes[max_idx].amp)
            max_idx = i;
    }

    out_notes.push_back(candidate_notes[max_idx]);
}

// TODO: Could be optimized by returning first peak, as the peak picking algorithms sorts the peaks
void HighRes::get_lowest_peak(NoteSet &out_notes, const NoteSet &candidate_notes) {
    const int n_notes = candidate_notes.size();
    if(n_notes == 0)
        return;
    else if(n_notes == 1) {
        out_notes.push_back(candidate_notes[0]);
        return;
    }

    int low_idx = 0;
    for(int i = 1; i < n_notes; i++) {
        if(candidate_notes[i].freq < candidate_notes[low_idx].freq)
            low_idx = i;
    }

    out_notes.push_back(candidate_notes[low_idx]);
}

void HighRes::get_likeliest_note(NoteSet &out_notes, const NoteSet &candidate_notes) {
    const int n_notes = candidate_notes.size();
    if(n_notes == 0)
        return;
    else if(n_notes == 1 && candidate_notes[0].amp > 0.0) {
        out_notes.push_back(candidate_notes[0]);
        return;
    }

    std::vector<int> n_harmonics(n_notes, 0);
    for(int i = 0; i < n_notes; i++) {
        for(int j = i + 1; j < n_notes; j++) {
            const double detected_freq = candidate_notes[j].freq;
            const double theoretical_freq = candidate_notes[i].freq * round(candidate_notes[j].freq / candidate_notes[i].freq);
            const double cent_error = 1200.0 * log2(detected_freq / theoretical_freq);
            if(cent_error > -OVERTONE_ERROR && cent_error < OVERTONE_ERROR)
                n_harmonics[i]++;
        }
    }

    int max_idx = 0;
    for(int i = 1; i < n_notes; i++) {
        if(n_harmonics[i] > n_harmonics[max_idx])
            max_idx = i;
    }

    // Filter noise
    if(candidate_notes[max_idx].amp < 0.0)
        return;

    out_notes.push_back(candidate_notes[max_idx]);
}


void HighRes::perform(float *const input_buffer, NoteEvents &note_events) {
    max_norm = 0.0;

    // Apply window function to minimize spectral leakage
    for(int i = 0; i < FRAME_SIZE; i++)
        input_buffer[i] *= window_func[i];
    perf.push_time_point("Applied window function");

    // Do the actual transform
    fftwf_execute(p);
    perf.push_time_point("Fourier transformed");

    // Calculate amplitude of every frequency component
    double norms[(FRAME_SIZE / 2) + 1];
    double power;
    calc_norms(out, norms, (FRAME_SIZE / 2) + 1, max_norm, power);
    perf.push_time_point("Norms calculated");

    // Compute Gaussian envelope
    double envelope[(FRAME_SIZE / 2) + 1];
    calc_envelope(norms, envelope);

    // Find peaks based on envelope
    std::vector<int> peaks;
    if(power > POWER_THRESHOLD)
        envelope_peaks(norms, envelope, peaks);

    // Interpolate peak locations
    NoteSet candidate_notes;
    interpolate_peaks(candidate_notes, norms, peaks);

    // Extract played note from the peaks
    NoteSet noteset;  // Noteset, so polyphony can easily be supported
    // get_loudest_peak(noteset, candidate_notes);
    // get_lowest_peak(noteset, candidate_notes);
    get_likeliest_note(noteset, candidate_notes);

    // note_events.clear();
    if(noteset.size() > 0)
        note_events.push_back(NoteEvent(noteset[0], /*(double)FRAME_SIZE / (double)SAMPLE_RATE,*/ 0.0));
        // note_events.push_back(NoteEvent(noteset[0], (double)FRAME_SIZE / (double)SAMPLE_RATE, ((double)FRAME_SIZE / (double)SAMPLE_RATE) / 2.0));


    // Graphics
    if constexpr(!HEADLESS) {
        HighResGraphics *highres_graphics = static_cast<HighResGraphics *>(estimator_graphics);

        Spectrum &spectrum = highres_graphics->get_spectrum();
        spectrum.clear();

        Spectrum &envelope_spectrum = highres_graphics->get_envelope();
        envelope_spectrum.clear();

        // Start at i = 1 to skip rendering DC offset (envelope has no DC offset, so do first explicitly)
        envelope_spectrum.add_data(0.0, envelope[0], 0.0);
        for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++) {
            spectrum.add_data(i * ((double)SAMPLE_RATE / (double)FRAME_SIZE), norms[i], (double)SAMPLE_RATE / (double)FRAME_SIZE);
            envelope_spectrum.add_data(i * ((double)SAMPLE_RATE / (double)FRAME_SIZE), envelope[i], 0.0);
        }
        spectrum.sort();
        envelope_spectrum.sort();


        std::vector<double> &f_peaks = highres_graphics->get_peaks();
        f_peaks.clear();
        for(int peak_bin : peaks)
            f_peaks.push_back(peak_bin * ((double)SAMPLE_RATE / (double)FRAME_SIZE));
        std::sort(f_peaks.begin(), f_peaks.end());
    }
}
