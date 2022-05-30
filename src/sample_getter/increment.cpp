#include "increment.h"

#include "error.h"
#include "config/transcription.h"

#include <cstring>


Increment::Increment(const int input_buffer_size) : SampleGetter(input_buffer_size) {
    played_samples = 1;
}

Increment::~Increment() {

}


SampleGetters Increment::get_type() const {
    return SampleGetters::increment;
}


bool Increment::is_audio_recording_device() const {
    return true;
}


void Increment::calc_and_paste_nonblocking_overlap(float *&in, int &n_samples) {
    static int samples_queued = 1;

    const int min_overlap_samples = std::max((int)((double)n_samples * MIN_NONBLOCK_OVERLAP_RATIO), 1);
    const int max_overlap_samples = std::min((int)((double)n_samples * MAX_NONBLOCK_OVERLAP_RATIO), n_samples - 1);

    const int n_overlap = std::clamp(n_samples - samples_queued, min_overlap_samples, max_overlap_samples);
    debug("Overlapping " + STR(n_overlap) + " samples");

    // Paste the end of overlap_buffer to start of 'in'
    memcpy(in, overlap_buffer + (n_samples - n_overlap), n_overlap * sizeof(float));

    // Calculate the remainder of the buffer
    in += n_overlap;
    n_samples -= n_overlap;

    samples_queued += 1;
}


void Increment::copy_nonblocking_overlap(float *const in, const int n_samples) {
    memcpy(overlap_buffer, in, n_samples * sizeof(float));

    // Possible optimization if n_samples is the same every call
    // const int max_overlap_samples = std::min((int)((double)n_samples * MAX_NONBLOCK_OVERLAP_RATIO), n_samples - 1);
    // memcpy(overlap_buffer + (n_samples - max_overlap_samples), in + (n_samples - max_overlap_samples), max_overlap_samples * sizeof(float));
}


int Increment::get_frame(float *const in, const int n_samples) {
    int overlap_n_samples = n_samples;
    float *overlap_in = in;
    if constexpr(DO_OVERLAP)
        calc_and_paste_overlap(overlap_in, overlap_n_samples);
    else if constexpr(DO_OVERLAP_NONBLOCK)
        calc_and_paste_nonblocking_overlap(overlap_in, overlap_n_samples);


    for(int i = 0; i < overlap_n_samples; i++)
        overlap_in[i] = played_samples + i;

    played_samples += overlap_n_samples;


    if constexpr(DO_OVERLAP)
        copy_overlap(in, n_samples);
    else if constexpr(DO_OVERLAP_NONBLOCK)
        copy_nonblocking_overlap(in, n_samples);

    return overlap_n_samples;
}
