
#ifndef PROGRAM_H
#define PROGRAM_H


#include "graphics.h"
#include "config.h"

#include "sample_getter/sample_getters.h"

#include <SDL2/SDL.h>

#include <fstream>


class Program {
    public:
        Program(Graphics *const _g, SDL_AudioDeviceID *const in_dev, SDL_AudioDeviceID *const out_dev);
        ~Program();

        void main_loop();

        void resize(const int w, const int h);


    private:
        Graphics *const graphics;

        SDL_AudioDeviceID *in_dev;
        SDL_AudioDeviceID *out_dev;

        SampleGetter *sample_getter;

        bool mouse_clicked;
        int mouse_x, mouse_y;

        std::fstream output_stream;


        void handle_sdl_events();
};


#endif  // PROGRAM_H
