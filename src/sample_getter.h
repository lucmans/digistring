
#ifndef SAMPLE_GETTER_H
#define SAMPLE_GETTER_H


#include "note.h"

#include <SDL2/SDL.h>


enum class SoundSource {
    audio_in, generate_sine, generate_note, file
};


class SampleGetter {
    public:
        SampleGetter(SDL_AudioDeviceID *const _in);
        ~SampleGetter();

        SoundSource get_sound_source() const;

        void add_generated_wave_freq(const double d_freq);

        void set_note(const Note &new_note);
        void note_up();
        void note_down();

        void get_frame(float *const in, const int n_samples);


    private:
        SDL_AudioDeviceID *in_dev;

        SoundSource sound_source;
        double generated_wave_freq;
        Note generated_note;


        void read_frame_float32_audio_device(float *const in, const int n_samples);
        void read_frame_int32_audio_device(float *const in, const int n_samples);
};


#endif  // SAMPLE_GETTER_H
