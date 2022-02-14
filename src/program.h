
#ifndef PROGRAM_H
#define PROGRAM_H


#include "graphics.h"
#include "results_file.h"
#include "config.h"

#include "note.h"
#include "estimators/estimators.h"
#include "sample_getter/sample_getters.h"

#include <SDL2/SDL.h>

#include <chrono>


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

        Estimator *estimator;
        float *input_buffer;
        int input_buffer_n_samples;

        SampleGetter *sample_getter;
        bool audio_in;

        // Frame limiting (graphics)
        std::chrono::duration<double, std::milli> frame_time;
        std::chrono::steady_clock::time_point prev_frame;

        // Printing clicked location info (graphics)
        bool mouse_clicked;
        int mouse_x, mouse_y;

        // Output results file
        ResultsFile *results_file;

        // DEBUG
        int lag;  // ms

        // Arpeggiator (easter egg)
        bool plus_held_down, minus_held_down;
        std::chrono::duration<double, std::milli> note_change_time;
        std::chrono::steady_clock::time_point prev_note_change;


        void playback_audio();

        // These functions should only be called if settings.output_file is true
        void write_result_header();
        void write_results(const NoteEvents &note_events);

        void print_results(const NoteEvents &note_events);

        void update_graphics(const NoteEvents &note_events);

        void arpeggiate();

        void handle_sdl_events();
};


#endif  // PROGRAM_H
