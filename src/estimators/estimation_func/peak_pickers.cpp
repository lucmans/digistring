#include "peak_pickers.h"

#include "config/transcription.h"

#include <cmath>
#include <vector>


void all_max(const double norms[], const int n_norms, std::vector<int> &peaks) {
    for(int i = 1; i < n_norms - 1; i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1])
            peaks.push_back(i);
    }

    // Filter quiet peaks
    for(size_t i = peaks.size(); i > 0; i--) {
        if(norms[peaks[i - 1]] < PEAK_THRESHOLD)
            peaks.erase(peaks.begin() + (i - 1));
    }
}

// With signal to noise filter
void all_max(const double norms[], const int n_norms, std::vector<int> &peaks, const int max_norm) {
    for(int i = 1; i < n_norms - 1; i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1] && norms[i] > max_norm * SIGNAL_TO_NOISE_FILTER)
            peaks.push_back(i);
    }

    // Filter quiet peaks
    for(size_t i = peaks.size(); i > 0; i--) {
        if(norms[peaks[i - 1]] < PEAK_THRESHOLD)
            peaks.erase(peaks.begin() + (i - 1));
    }
}


void envelope_peaks(const double norms[], const double envelope[], const int n_norms, std::vector<int> &peaks) {
    for(int i = 5; i < n_norms - 1; i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1]  // A local maximum
           && norms[i] > envelope[i]  // Higher than envelope
           && envelope[i] > ENVELOPE_MIN)  // Filter quiet peaks
            peaks.push_back(i);
    }
}

void envelope_peaks(const double norms[], const double envelope[], const int n_norms, std::vector<int> &peaks, const int max_norm) {
    for(int i = 5; i < n_norms - 1; i++) {
        if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1]  // A local maximum
           && norms[i] > envelope[i]  // Higher than envelope
           && envelope[i] > ENVELOPE_MIN  // Filter quiet peaks
           && norms[i] > max_norm * SIGNAL_TO_NOISE_FILTER)
            peaks.push_back(i);
    }
}


void min_dy_peaks(const double norms[], const int n_norms, std::vector<int> &peaks) {
    bool was_peak = false;  // If last extreme value was a peak
    int extreme_value_idx = 0;

    for(int i = 1; i < n_norms - 1; i++) {
        if(was_peak) {
            // If previous extreme value was a peak, look for a valley
            if(norms[i - 1] > norms[i] && norms[i] < norms[i + 1]) {
                // If difference in y is significant enough
                was_peak = false;
                extreme_value_idx = i;
            }
        }
        else {
            // If previous extreme value was a valley, look for a peak
            if(norms[i - 1] < norms[i] && norms[i] > norms[i + 1]) {
                // If difference in y is significant enough
                if(abs(norms[extreme_value_idx] - norms[i]) > MIN_PEAK_DY) {
                    peaks.push_back(i);
                }
                was_peak = true;
                extreme_value_idx = i;
            }
        }
    }
}
