#include "sample_getter.h"

#include "config/audio.h"
#include "config/transcription.h"

#include <cstring>  // memset(), memcpy()
#include <algorithm>  // std::clamp()


SampleGetter::SampleGetter() {
    played_samples = 0;

    memset(overlap_buffer, 0, MAX_FRAME_SIZE * sizeof(float));
}

SampleGetter::~SampleGetter() {

}


unsigned long SampleGetter::get_played_samples() const {
    return played_samples;
}


double SampleGetter::get_played_time() const {
    return (double)played_samples / (double)SAMPLE_RATE;
}


void SampleGetter::calc_and_paste_overlap(float *&in, int &n_samples) const {
    // Clamp so at least one sample is overlapped or kept between frames
    const int n_overlap = std::clamp((int)(n_samples * OVERLAP_RATIO), 1, n_samples - 1);

    // Paste the overlap to start of 'in'
    memcpy(in, overlap_buffer, n_overlap * sizeof(float));

    // Calculate the remainder of the buffer
    in += n_overlap;
    n_samples -= n_overlap;
}


void SampleGetter::copy_overlap(float *const in, const int n_samples) {
    // Clamp so at least one sample is overlapped or kept between frames
    const int n_overlap = std::clamp((int)(n_samples * OVERLAP_RATIO), 1, n_samples - 1);

    memcpy(overlap_buffer, in + (n_samples - n_overlap), n_overlap * sizeof(float));
}
