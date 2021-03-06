#include "wave_generator.h"

#include "error.h"

#include "config/audio.h"
#include "config/transcription.h"

#include <cmath>
#include <iostream>


static constexpr double MIN_FREQ = 1.0;


WaveGenerator::WaveGenerator(const int input_buffer_size, const double freq) : SampleGetter(input_buffer_size) {
    generated_wave_freq = freq;

    last_phase = 0.0;
}

WaveGenerator::~WaveGenerator() {

}


SampleGetters WaveGenerator::get_type() const {
    return SampleGetters::wave_generator;
}


void WaveGenerator::pitch_up() {
    generated_wave_freq += D_FREQ;

    info("Playing sine wave of " + STR(generated_wave_freq) + " Hz");
}

void WaveGenerator::pitch_down() {
    generated_wave_freq -= D_FREQ;

    if(generated_wave_freq < MIN_FREQ) {
        warning("Can't set frequency < " + STR(MIN_FREQ));
        hint("MIN_FREQ is configurable in src/sample_getter/wave_generator.cpp");
        generated_wave_freq = MIN_FREQ;
    }

    info("Playing sine wave of " + STR(generated_wave_freq) + " Hz");
}


int WaveGenerator::get_frame(float *const in, const int n_samples) {
    int overlap_n_samples = n_samples;
    float *overlap_in = in;
    if constexpr(DO_OVERLAP)
        calc_and_paste_overlap(overlap_in, overlap_n_samples);


    const double offset = (last_phase * ((double)SAMPLE_RATE / generated_wave_freq));
    for(int i = 0; i < overlap_n_samples; i++)
        overlap_in[i] = sinf((2.0 * M_PI * ((double)i + offset) * generated_wave_freq) / (double)SAMPLE_RATE);

    last_phase = fmod(last_phase + (generated_wave_freq / ((double)SAMPLE_RATE / (double)overlap_n_samples)), 1.0);

    played_samples += overlap_n_samples;


    if constexpr(DO_OVERLAP)
        copy_overlap(in, n_samples);

    return overlap_n_samples;
}
