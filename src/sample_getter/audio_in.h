
#ifndef AUDIO_IN
#define AUDIO_IN


#include "sample_getter.h"

#include <SDL2/SDL.h>


class AudioIn : public SampleGetter {
    public:
        AudioIn(SDL_AudioDeviceID *const _in);
        ~AudioIn() override;

        SampleGetters get_type() const override;

        void read_frame_float32_audio_device(float *const in, const int n_samples);
        void read_frame_int32_audio_device(float *const in, const int n_samples);

        void calc_and_paste_nonblocking_overlap(float *&in, int &n_samples, const int bits_per_sample);
        void copy_nonblocking_overlap(float *const in, const int n_samples);

        void get_frame(float *const in, const int n_samples);


    private:
        SDL_AudioDeviceID *in_dev;

        // float overlap_buffer[MAX_FRAME_SIZE];  // Declared in SampleGetter
        // int last_overlap_size;
};


#endif  // AUDIO_IN
