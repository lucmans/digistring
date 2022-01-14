
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

        void get_frame(float *const in, const int n_samples);


    private:
        SDL_AudioDeviceID *in_dev;
};


#endif  // AUDIO_IN
