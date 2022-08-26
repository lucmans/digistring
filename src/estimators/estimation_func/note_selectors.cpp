#include "note_selectors.h"

#include "note.h"

#include "config/transcription.h"

#include <vector>


void get_loudest_peak(NoteSet &out_notes, const NoteSet &peaks) {
    const int n_peaks = peaks.size();
    if(n_peaks == 0)
        return;
    else if(n_peaks == 1) {
        out_notes.push_back(peaks[0]);
        return;
    }

    int max_idx = 0;
    for(int i = 1; i < n_peaks; i++) {
        if(peaks[i].amp > peaks[max_idx].amp)
            max_idx = i;
    }

    out_notes.push_back(peaks[max_idx]);
}


// TODO: Could be optimized by returning first peak, as the peak picking algorithms sorts the peaks
void get_lowest_peak(NoteSet &out_notes, const NoteSet &peaks) {
    const int n_peaks = peaks.size();
    if(n_peaks == 0)
        return;
    else if(n_peaks == 1) {
        out_notes.push_back(peaks[0]);
        return;
    }

    int low_idx = 0;
    for(int i = 1; i < n_peaks; i++) {
        if(peaks[i].freq < peaks[low_idx].freq)
            low_idx = i;
    }

    out_notes.push_back(peaks[low_idx]);
}


void get_most_overtones(NoteSet &out_notes, const NoteSet &peaks) {
    const int n_peaks = peaks.size();
    if(n_peaks == 0)
        return;
    else if(n_peaks == 1 && peaks[0].amp > 0.0) {
        out_notes.push_back(peaks[0]);
        return;
    }

    std::vector<int> n_harmonics(n_peaks, 0);
    for(int i = 0; i < n_peaks; i++) {
        for(int j = i + 1; j < n_peaks; j++) {
            const double detected_freq = peaks[j].freq;
            const double theoretical_freq = peaks[i].freq * round(peaks[j].freq / peaks[i].freq);
            const double cent_error = 1200.0 * log2(detected_freq / theoretical_freq);
            // const double hz_error = abs(detected_freq - theoretical_freq);

            // if(hz_error > -OVERTONE_ERROR && hz_error < OVERTONE_ERROR)
            if(cent_error > -OVERTONE_ERROR && cent_error < OVERTONE_ERROR)
                n_harmonics[i]++;
        }
    }

    int max_idx = 0;
    for(int i = 1; i < n_peaks; i++) {
        if(n_harmonics[i] > n_harmonics[max_idx])
            max_idx = i;
    }

    // n overtone filter
    // if(n_harmonics[max_idx] < 2)
    //     return;

    // Filter noise
    if(peaks[max_idx].amp < 0.0)
        return;

    // Filter transients
    // if(peaks.size() > n_harmonics[max_idx] * 2)
    //     return;

    out_notes.push_back(peaks[max_idx]);
}

void get_most_overtones(NoteSet &out_notes, const NoteSet &peaks, NoteSet &out_note_peaks) {
    const int n_peaks = peaks.size();
    if(n_peaks == 0)
        return;
    else if(n_peaks == 1 && peaks[0].amp > 0.0) {
        out_notes.push_back(peaks[0]);
        out_note_peaks.push_back(peaks[0]);
        return;
    }

    std::vector<int> n_harmonics(n_peaks, 0);
    for(int i = 0; i < n_peaks; i++) {
        for(int j = i + 1; j < n_peaks; j++) {
            const double detected_freq = peaks[j].freq;
            const double theoretical_freq = peaks[i].freq * round(peaks[j].freq / peaks[i].freq);
            const double cent_error = 1200.0 * log2(detected_freq / theoretical_freq);

            if(cent_error > -OVERTONE_ERROR && cent_error < OVERTONE_ERROR)
                n_harmonics[i]++;
        }
    }

    int max_idx = 0;
    for(int i = 1; i < n_peaks; i++) {
        if(n_harmonics[i] > n_harmonics[max_idx])
            max_idx = i;
    }

    // Mark all "matched" peaks for graphics
    out_note_peaks.clear();
    out_note_peaks.push_back(peaks[max_idx]);
    for(int i = max_idx + 1; i < n_peaks; i++) {
        const double detected_freq = peaks[i].freq;
        const double theoretical_freq = peaks[max_idx].freq * round(peaks[i].freq / peaks[max_idx].freq);
        const double cent_error = 1200.0 * log2(detected_freq / theoretical_freq);
        if(cent_error > -OVERTONE_ERROR && cent_error < OVERTONE_ERROR)
            out_note_peaks.push_back(peaks[i]);
    }

    // n overtone filter
    // if(n_harmonics[max_idx] < 2)
    //     return;

    // Filter noise
    if(peaks[max_idx].amp < 0.0)
        return;

    // Filter transients
    // if(peaks.size() > n_harmonics[max_idx] * 2)
    //     return;

    out_notes.push_back(peaks[max_idx]);
}


void get_most_overtone_power(NoteSet &out_notes, const NoteSet &peaks) {
    const int n_peaks = peaks.size();
    if(n_peaks == 0)
        return;
    else if(n_peaks == 1 && peaks[0].amp > 0.0) {
        out_notes.push_back(peaks[0]);
        return;
    }

    std::vector<double> overtone_power(n_peaks, 0.0);
    for(int i = 0; i < n_peaks; i++) {
        for(int j = i + 1; j < n_peaks; j++) {
            const double detected_freq = peaks[j].freq;
            const double theoretical_freq = peaks[i].freq * round(peaks[j].freq / peaks[i].freq);
            const double cent_error = 1200.0 * log2(detected_freq / theoretical_freq);
            if(cent_error > -OVERTONE_ERROR && cent_error < OVERTONE_ERROR)
                overtone_power[i] += peaks[j].amp;
        }
    }

    int max_idx = 0;
    for(int i = 1; i < n_peaks; i++) {
        if(overtone_power[i] > overtone_power[max_idx])
            max_idx = i;
    }

    // Filter noise
    if(peaks[max_idx].amp < 0.0)
        return;

    // Filter transients
    // if(peaks.size() > overtone_power[max_idx] * 2)
    //     return;

    out_notes.push_back(peaks[max_idx]);
}
