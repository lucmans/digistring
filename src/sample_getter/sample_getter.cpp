#include "sample_getter.h"

#include "error.h"

#include "config/audio.h"
#include "config/transcription.h"

#include <cstring>  // memset(), memcpy()
#include <algorithm>  // std::clamp() std::fill_n()


SampleGetter::SampleGetter(const int input_buffer_size) {
    played_samples = 0;

    overlap_buffer = nullptr;
    overlap_buffer_size = 0;
    if constexpr(DO_OVERLAP || DO_OVERLAP_NONBLOCK) {
        if constexpr(DO_OVERLAP)
            overlap_buffer_size = input_buffer_size * OVERLAP_RATIO;
        else if constexpr(DO_OVERLAP_NONBLOCK)
            overlap_buffer_size = input_buffer_size - MIN_NEW_SAMPLES_NONBLOCK;

        try {
            overlap_buffer = new float[overlap_buffer_size];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to create overlap buffer");
            hint("Setting a lower value for OVERLAP_RATIO in config/transcription.h or disable overlapping completely");
            exit(EXIT_FAILURE);
        }

        std::fill_n(overlap_buffer, overlap_buffer_size, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles
    }
}

SampleGetter::~SampleGetter() {
    delete[] overlap_buffer;
}


long SampleGetter::get_played_samples() const {
    return played_samples;
}


double SampleGetter::get_played_time() const {
    return (double)played_samples / (double)SAMPLE_RATE;
}


void SampleGetter::calc_and_paste_overlap(float *&in, int &n_samples) const {
    // Clamp so at least one sample is overlapped or kept between frames
    const int n_overlap = std::clamp((int)(n_samples * OVERLAP_RATIO), 1, n_samples - 1);

    // DEBUG
    if(n_overlap > overlap_buffer_size) {
        error("Calculated number of overlapping samples is bigger that overlap buffer");
        hint("Size of input buffer may never increase");
        exit(EXIT_FAILURE);
    }
    else if(n_overlap != overlap_buffer_size)
        warning("overlap_buffer_size != n_overlap");

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
