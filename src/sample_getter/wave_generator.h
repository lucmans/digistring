#ifndef DIGISTRING_SAMPLE_GETTER_WAVE_GENERATOR_H
#define DIGISTRING_SAMPLE_GETTER_WAVE_GENERATOR_H


#include "sample_getter.h"


static const double D_FREQ = 5.0;


class WaveGenerator : public SampleGetter {
    public:
        WaveGenerator(const double freq);
        ~WaveGenerator() override;

        SampleGetters get_type() const override;

        void pitch_up() override;
        void pitch_down() override;

        void get_frame(float *const in, const int n_samples);


    private:
        double generated_wave_freq;

        double last_phase;
};


#endif  // DIGISTRING_SAMPLE_GETTER_WAVE_GENERATOR_H
