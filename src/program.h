
#ifndef PROGRAM_H
#define PROGRAM_H


#include "graphics.h"
#include "config.h"

#include <fftw3.h>


class Program {
    public:
        Program(Graphics *const _g, SDL_AudioDeviceID *const in_dev, SDL_AudioDeviceID *const out_dev);
        ~Program();

        void main_loop();

        void resize(const int w, const int h);


    private:
        Graphics *graphics;

        SDL_AudioDeviceID *in_dev;
        SDL_AudioDeviceID *out_dev;

        float *in;
        fftwf_complex *out;
        fftwf_plan p;

        double window_func[FRAME_SIZE];

        double generated_wave_freq;


        void handle_sdl_events();

        void read_frame(float *const in);
        void transcribe();
};


#endif  // PROGRAM_H
