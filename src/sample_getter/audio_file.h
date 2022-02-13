
#ifndef AUDIO_FILE
#define AUDIO_FILE


#include "sample_getter.h"

#include <SDL2/SDL.h>

#include <string>


class AudioFile : public SampleGetter {
    public:
        AudioFile(const std::string &file);
        ~AudioFile() override;

        SampleGetters get_type() const override;

        void get_frame(float *const in, const int n_samples);


    private:
        float *wav_buffer;
        int wav_buffer_samples;
        SDL_AudioSpec wav_spec;
};


#endif  // AUDIO_FILE
