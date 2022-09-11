#ifndef DIGISTRING_PROGRAM_H
#define DIGISTRING_PROGRAM_H


#include "graphics.h"
#include "results_file.h"
#include "midi_out.h"

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
        int synth_buffer_n_samples;
        double volume;

        float *playback_buffer;  // Only used during stereo split playback-synth
        int playback_buffer_n_samples;

        // Frame limiting (graphics)
        std::chrono::duration<double, std::milli> frame_time;
        std::chrono::steady_clock::time_point prev_frame;

        // Printing clicked location info (graphics)
        bool mouse_clicked;
        int mouse_x, mouse_y;

        // Output results file
        ResultsFile *results_file;

        MidiOut *midi_out;

        // Arpeggiator (easter egg)
        bool plus_held_down, minus_held_down;
        std::chrono::duration<double, std::milli> note_change_time;
        std::chrono::steady_clock::time_point prev_note_change;


        // This function should only be called if cli_args.playback is true
        // Queues samples in audio out buffer, but doesn't block (is done by sync_with_audio())
        void playback_audio(const int new_samples);

        // These functions should only be called if cli_args.output_file is true
        void write_result_header();
        void write_results(const NoteEvents &note_events, const int new_samples);

        void print_results(const NoteEvents &note_events) const;

        // If less than input_buffer_n_samples is retrieved, only the NoteEvents regarding the first 'new_samples' samples are relevant, as the rest is "overwritten" in the next cycle
        // Adjust the note events to reflect this smaller frame (Estimator doesn't know about overlap, so gives full frame lengths to note events)
        static void adjust_events(NoteEvents &events, const int n_frame_samples, const int new_samples);

        static void slowdown(NoteEvents &events, int &new_samples);

        // This function should only be called if HEADLESS is false
        bool update_graphics(const NoteEvents &note_events);

        // Queues samples in audio out buffer, but doesn't block (is done by sync_with_audio())
        void synthesize_audio(const NoteEvents &notes, const int new_samples);

        void play_split_audio(const int new_samples);

        // Wait till one frame is left in systems audio out buffer (needed when fetching samples is faster than playing)
        // In case of no audio out, simulate the behavior by timing duration between calls and waiting the appropriate time
        // This effectively syncs program with current audio out
        void sync_with_audio(const int new_samples);

        // Easter egg arpeggiator
        void arpeggiate();

        void handle_sdl_events();
};


#endif  // DIGISTRING_PROGRAM_H
