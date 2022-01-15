
#include "wave_generator.h"

#include "../config.h"
#include "../error.h"

#include <cmath>
#include <iostream>


WaveGenerator::WaveGenerator(const double freq) {
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

    std::cout << "Playing sine wave of " << generated_wave_freq << " Hz" << std::endl;
}

void WaveGenerator::pitch_down() {
    generated_wave_freq -= D_FREQ;

    std::cout << "Playing sine wave of " << generated_wave_freq << " Hz" << std::endl;
}


void WaveGenerator::get_frame(float *const in, const int n_samples) {
    for(int i = 0; i < n_samples; i++) {
        const double offset = (last_phase * ((double)SAMPLE_RATE / generated_wave_freq));
        in[i] = sinf((2.0 * M_PI * ((double)i + offset) * generated_wave_freq) / (double)SAMPLE_RATE);
    }
    last_phase = fmod(last_phase + (generated_wave_freq / ((double)SAMPLE_RATE / (double)n_samples)), 1.0);

    played_samples += n_samples;
}
