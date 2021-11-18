
#ifndef SAMPLE_GETTER_H
#define SAMPLE_GETTER_H


#include <SDL2/SDL.h>


enum class SoundSource {
    audio_in, generate_sine//, file
};


class SampleGetter {
    public:
        SampleGetter(SDL_AudioDeviceID *const _in);
        ~SampleGetter();

        void add_generated_wave_freq(const double d_freq);

        void get_frame(float *const in, const int n_samples);


    private:
        SDL_AudioDeviceID *in_dev;

        SoundSource sound_source;
        double generated_wave_freq;


        void read_frame_float32_audio_device(float *const in, const int n_samples);
        void read_frame_int32_audio_device(float *const in, const int n_samples);
};


#endif  // SAMPLE_GETTER_H
