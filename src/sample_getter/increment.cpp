
#include "increment.h"

#include "config/transcription.h"


Increment::Increment() {
    played_samples = 1;
}

Increment::~Increment() {

}


SampleGetters Increment::get_type() const {
    return SampleGetters::increment;
}


void Increment::get_frame(float *const in, const int n_samples) {
    int overlap_n_samples = n_samples;
    float *overlap_in = in;
    if constexpr(DO_OVERLAP)
        calc_and_paste_overlap(overlap_in, overlap_n_samples);


    for(int i = 0; i < overlap_n_samples; i++)
        overlap_in[i] = played_samples + i;

    played_samples += overlap_n_samples;


    if constexpr(DO_OVERLAP)
        copy_overlap(in, n_samples);
}
