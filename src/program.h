#ifndef DIGISTRING_PROGRAM_H
#define DIGISTRING_PROGRAM_H


#include "graphics.h"
#include "results_file.h"

#include "note.h"
#include "estimators/estimators.h"
#include "sample_getter/sample_getters.h"
#include "synth/synths.h"

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

        Synth *synth;
        float *synth_buffer;

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


        // This function should only be called if cli_args.playback is true
        void playback_audio(const int new_samples);

        // These functions should only be called if cli_args.output_file is true
        void write_result_header();
        void write_results(const NoteEvents &note_events, const int new_samples);

        void print_results(const NoteEvents &note_events);

        // If less than input_buffer_n_samples is retrieved, only the NoteEvents regarding the first 'new_samples' samples are relevant, as the rest is "overwritten" in the next cycle
        // Adjust the note events to reflect this smaller frame (Estimator doesn't know about overlap, so gives full frame lengths to note events)
        void adjust_events(NoteEvents &events, const int new_samples);

        // This function should only be called if HEADLESS is false
        void update_graphics(const NoteEvents &note_events);

        void synthesize_audio(const NoteEvents &notes, const int new_samples);

        void arpeggiate();

        void handle_sdl_events();
};


#endif  // DIGISTRING_PROGRAM_H
