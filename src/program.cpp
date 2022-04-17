#include "program.h"

#include "graphics.h"
#include "results_file.h"
#include "performance.h"
#include "quit.h"
#include "error.h"

#include "config/cli_args.h"
#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"
#include "config/results_file.h"

#include "estimators/estimators.h"
#include "sample_getter/sample_getters.h"

#include <SDL2/SDL.h>

#include <iomanip>  // std::setw()
#include <chrono>
#include <thread>  // sleep


Program::Program(Graphics *const _g, SDL_AudioDeviceID *const _in, SDL_AudioDeviceID *const _out) : graphics(_g) {
    in_dev = _in;
    out_dev = _out;

    prev_frame = std::chrono::steady_clock::now();

    // We let the estimator create the input buffer for optimal size and better alignment
    input_buffer = NULL;
    input_buffer_n_samples = -1;
    // estimator = new Tuned(input_buffer, input_buffer_n_samples);
    estimator = new HighRes(input_buffer, input_buffer_n_samples);
    if(input_buffer == NULL) {
        error("Estimator did not create an input buffer");
        exit(EXIT_FAILURE);
    }
    if(input_buffer_n_samples == -1) {
        error("Estimator did not set number of samples in input buffer");
        exit(EXIT_FAILURE);
    }

    audio_in = false;
    if(cli_args.generate_sine) {
        // info("Playing generate_sine");
        sample_getter = new WaveGenerator(input_buffer_n_samples, cli_args.generate_sine_freq);
    }
    else if(cli_args.generate_note) {
        // info("Playing generate_note");
        sample_getter = new NoteGenerator(input_buffer_n_samples, cli_args.generate_note_note);
    }
    else if(cli_args.play_file) {
        // info("Playing '" + cli_args.play_file_name + "'");
        sample_getter = new AudioFile(input_buffer_n_samples, cli_args.play_file_name);
    }
    else {
        // info("Sourcing audio from computer audio input");
        sample_getter = new AudioIn(input_buffer_n_samples, in_dev);
        audio_in = true;
    }

    // DEBUG: Sanity check
    if(cli_args.playback && cli_args.synth) {
        error("Can't both play back input audio and synthesize audio based on estimation");
        exit(EXIT_FAILURE);
    }
    if(cli_args.synth) {
        try {
            synth_buffer = new float[input_buffer_n_samples];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to allocate synth_buffer (" + STR(e.what()) + ")");
            hint("Try using an Estimator with a smaller input buffer size or don't synthesize sound");
            exit(EXIT_FAILURE);
        }
        synth = synth_factory(cli_args.synth_type);
    }

    mouse_clicked = false;

    if(cli_args.output_file)
        results_file = new ResultsFile(cli_args.output_filename);

    lag = 0;

    note_change_time = std::chrono::duration<double>(NOTE_TIME + 1);
}

Program::~Program() {
    if(cli_args.output_file)
        delete results_file;

    if(cli_args.synth) {
        delete[] synth_buffer;
        delete synth;
    }

    delete sample_getter;
    delete estimator;
}


void Program::main_loop() {
    // Unpause audio devices so that samples are collected/played
    if(audio_in)
        SDL_PauseAudioDevice(*in_dev, 0);

    if(cli_args.playback || cli_args.synth)
        SDL_PauseAudioDevice(*out_dev, 0);

    if(cli_args.output_file) {
        write_result_header();
        results_file->start_array("note events");  // Stopped after while loop, so only note events can be pushed from now on
    }

    while(!poll_quit()) {
        perf.clear_time_points();
        perf.push_time_point("Start");

        // Check for keyboard/mouse/window/OS events
        handle_sdl_events();
        if(poll_quit())
            break;

        // DEBUG
        if(lag != 0) {  // Lag might have been increased by in handle_sdl_events()
            debug("Lagging for " + STR(lag) + " ms");
            std::this_thread::sleep_for(std::chrono::milliseconds(lag));
            lag = 0;
        }

        perf.push_time_point("Handled SDL events");

        // Read a frame
        const int new_samples = sample_getter->get_frame(input_buffer, input_buffer_n_samples);  // TODO: Don't block on this call (or still handle SDL events during)
        perf.push_time_point("Got frame full of audio samples");

        // Play it back to the user if chosen
        if(cli_args.playback)
            playback_audio(new_samples);

        // Send frame to estimator
        NoteEvents note_events;
        estimator->perform(input_buffer, note_events);
        perf.push_time_point("Performed estimation");

        // Arg parser disallows both cli_args.playback and cli_args.synth to be true (sanity checked in constructor)
        if(cli_args.synth)
            synthesize_audio(note_events/*, new_samples*/);

        // Print note estimation to CLI
        // print_results(note_events);

        // Write estimation to output file
        if(cli_args.output_file)
            write_results(note_events);

        // Graphics
        if constexpr(!HEADLESS)
            update_graphics(note_events);

        // Print performance information to CLI
        if(cli_args.output_performance)
            std::cout << perf << std::endl;

        // Arpeggiator easter egg
        if constexpr(ENABLE_ARPEGGIATOR)
            arpeggiate();
    }

    if(cli_args.output_file) {
        // Write silent note event to explicitly stop the last note
        results_file->start_dict();
        results_file->write_double("t (s)", sample_getter->get_played_time());
        results_file->write_null("note");
        results_file->write_null("frequency");
        results_file->write_null("amplitude");
        results_file->write_null("error");
        results_file->write_null("midi_number");
        results_file->stop_dict();

        // Note events array
        results_file->stop_array();
    }
}


void Program::resize(const int w, const int h) {
    graphics->resize_window(w, h);
}


void Program::playback_audio(const int new_samples) {
    if constexpr(PRINT_AUDIO_UNDERRUNS)
        if(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) == 0)
            warning("Audio underrun; no audio left to play");

    if(SDL_QueueAudio(*out_dev, input_buffer + (input_buffer_n_samples - new_samples), new_samples * sizeof(float))) {
        error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // DEBUG: If the while() below works correctly, the out buffer should never be filled faster than it is played
    if(audio_in && SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) > (unsigned int)input_buffer_n_samples * 1.9) {
        warning("Audio overrun (too much audio to play); clearing buffer...");
        SDL_ClearQueuedAudio(*out_dev);
    }

    // Wait till one frame is left in systems audio out buffer (needed when fetching samples is faster than playing)
    while(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) >= (unsigned int)input_buffer_n_samples && !poll_quit())
        handle_sdl_events();
}


void Program::write_result_header() {
    results_file->write_int("Sample rate (Hz)", SAMPLE_RATE);
    results_file->write_int("Input buffer size (samples)", input_buffer_n_samples);
    results_file->write_double("Input buffer time (ms)", ((double)input_buffer_n_samples * 1000.0) / (double)SAMPLE_RATE);
    results_file->write_double("Fourier bin size (Hz)", (double)SAMPLE_RATE / (double)input_buffer_n_samples);

    if(DO_OVERLAP)
        results_file->write_double("Overlap ratio", OVERLAP_RATIO);

    if(DO_OVERLAP_NONBLOCK) {
        results_file->write_int("Minimum samples overlap", MIN_NEW_SAMPLES_NONBLOCK);
        results_file->write_int("Maximum samples overlap", MAX_NEW_SAMPLES_NONBLOCK);
    }
}

void Program::write_results(const NoteEvents &note_events) {
    results_file->start_dict();

    const double start_frame_time = sample_getter->get_played_time() - ((double)input_buffer_n_samples / (double)SAMPLE_RATE);

    const int n_notes = note_events.size();
    if(n_notes == 0) {
        if constexpr(WRITE_SILENCE) {
            results_file->write_double("t (s)", start_frame_time);
            results_file->write_null("duration (s)");
            results_file->write_null("note");
            results_file->write_null("frequency");
            results_file->write_null("amplitude");
            results_file->write_null("error");
            results_file->write_null("midi_number");
        }
    }

    else if(n_notes == 1) {
        const std::string note = note_to_string_ascii(note_events[0].note);
        results_file->write_double("t (s)", start_frame_time + ((double)note_events[0].offset / (double)SAMPLE_RATE));
        results_file->write_double("duration (s)", (double)note_events[0].length / (double)SAMPLE_RATE);
        results_file->write_string("note", note);
        results_file->write_double("frequency", note_events[0].note.freq);
        results_file->write_double("amplitude", note_events[0].note.amp);
        results_file->write_double("error", note_events[0].note.error);
        results_file->write_int("midi_number", note_events[0].note.midi_number);
    }

    else  // n_notes > 1
        warning("Polyphonic output is not yet supported");  // TODO: Support

    results_file->stop_dict();
}


void Program::print_results(const NoteEvents &note_events) {
    if(note_events.size() == 0)
        return;

    if(note_events.size() > 1) {
        warning("Polyphonic result printing is not yet supported");  // TODO: Support
        return;
    }

    std::cout << note_events[0].note << "  (" << note_events[0].note.freq << " Hz, " << note_events[0].note.amp << " dB, " << note_events[0].note.error << " cent)" << "     \r" << std::flush;
}


void Program::update_graphics(const NoteEvents &note_events) {
    // Get the pointer to the graphics object here, as it is only valid till next call to Estimator::perform()
    const EstimatorGraphics *const estimator_graphics = estimator->get_estimator_graphics();
    perf.push_time_point("Graphics parsed data");

    // Render data
    frame_time = std::chrono::steady_clock::now() - prev_frame;
    if(frame_time.count() > 1000.0 / MAX_FPS) {
        graphics->set_clicked((mouse_clicked ? mouse_x : -1), mouse_y);

        if(audio_in)
            graphics->set_queued_samples(SDL_GetQueuedAudioSize(*in_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8));
        else if(cli_args.play_file)
            graphics->set_file_played_time((dynamic_cast<AudioFile *>(sample_getter))->current_time());

        const int n_notes = note_events.size();
        if(n_notes == 0)
            graphics->render_frame(nullptr, estimator_graphics);
        else if(n_notes == 1)
            graphics->render_frame(&note_events[0].note, estimator_graphics);
        else  // n_notes > 1
            warning("Polyphonic graphics not yet supported");  // TODO: Support

        perf.push_time_point("Frame rendered");

        prev_frame = std::chrono::steady_clock::now();
    }
}


// TODO: Use new_samples
void Program::synthesize_audio(const NoteEvents &notes/*, const int new_samples*/) {
    if constexpr(PRINT_AUDIO_UNDERRUNS)
        if(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) == 0)
            warning("Audio underrun; no audio left to play");

    // if constexpr(SLOWDOWN_ON_OVERLAP) {
    //     //
    // }
    // else {
    //     //
    // }

    synth->synthesize(notes, synth_buffer, input_buffer_n_samples);

    if(SDL_QueueAudio(*out_dev, synth_buffer, input_buffer_n_samples * sizeof(float))) {
        error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // DEBUG: If the while() below works correctly, the out buffer should never be filled faster than it is played
    if(audio_in && SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) > (unsigned int)input_buffer_n_samples * 1.9) {
        warning("Audio overrun (too much audio to play); clearing buffer...");
        SDL_ClearQueuedAudio(*out_dev);
    }

    // Wait till one frame is left in systems audio out buffer (needed when fetching samples is faster than playing)
    while(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) >= (unsigned int)input_buffer_n_samples && !poll_quit())
        handle_sdl_events();
}


void Program::arpeggiate() {
    note_change_time = std::chrono::steady_clock::now() - prev_note_change;
    if(note_change_time.count() > NOTE_TIME) {
        if(!plus_held_down && minus_held_down)
            sample_getter->pitch_down();
        else if(plus_held_down && !minus_held_down)
            sample_getter->pitch_up();

        prev_note_change = std::chrono::steady_clock::now();
    }
}


void Program::handle_sdl_events() {
    SDL_Event e;
    while(SDL_PollEvent(&e) && !poll_quit()) {
        switch(e.type) {
            case SDL_QUIT:
            case SDL_APP_TERMINATING:
                set_quit();
                break;

            case SDL_KEYDOWN:
                switch(e.key.keysym.sym) {
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        set_quit();
                        break;

                    case SDLK_MINUS:
                        if constexpr(ENABLE_ARPEGGIATOR)
                            minus_held_down = true;
                        else
                            sample_getter->pitch_down();
                        break;

                    case SDLK_EQUALS:
                        if constexpr(ENABLE_ARPEGGIATOR)
                            plus_held_down = true;
                        else
                            sample_getter->pitch_up();
                        break;

                    case SDLK_r:
                        graphics->reset_max_recorded_value();
                        synth->reset_max_amp();
                        break;

                    case SDLK_i:
                        graphics->toggle_show_info();
                        break;

                    case SDLK_t:
                        if(cli_args.playback)
                            SDL_ClearQueuedAudio(*out_dev);
                        break;

                    case SDLK_p:
                        estimator->next_plot_type();
                        break;

                    case SDLK_LEFTBRACKET:
                        graphics->add_max_display_frequency(-300.0);
                        break;

                    case SDLK_RIGHTBRACKET:
                        graphics->add_max_display_frequency(300.0);
                        break;

                    // case SDLK_f:
                    //     graphics->toggle_freeze_graph();
                    //     break;

                    case SDLK_s:
                        debug("Creating lag spike");
                        lag += 250;
                        break;
                }
                break;

            case SDL_KEYUP:
                if constexpr(ENABLE_ARPEGGIATOR) {
                    switch(e.key.keysym.sym) {
                        case SDLK_MINUS:
                            minus_held_down = false;
                            break;

                        case SDLK_EQUALS:
                            plus_held_down = false;
                            break;
                    }
                }
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(e.button.button == SDL_BUTTON_LEFT) {
                    mouse_clicked = true;
                    mouse_x = e.button.x;
                    mouse_y = e.button.y;
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if(e.button.button == SDL_BUTTON_LEFT)
                    mouse_clicked = false;
                break;

            case SDL_MOUSEMOTION:
                if(e.motion.state & SDL_BUTTON_LMASK) {
                    mouse_clicked = true;
                    mouse_x = e.motion.x;
                    mouse_y = e.motion.y;
                }
                break;

            case SDL_MOUSEWHEEL:
                // if(cli_args.play_file) {
                if(sample_getter->get_type() == SampleGetters::audio_file) {
                    AudioFile *audio_in_cast = dynamic_cast<AudioFile *>(sample_getter);
                    if(e.wheel.y > 0)
                        audio_in_cast->seek((int)(SECONDS_PER_SCROLL * SAMPLE_RATE * e.wheel.y));
                    else if(e.wheel.y < 0)
                        audio_in_cast->seek((int)(SECONDS_PER_SCROLL * SAMPLE_RATE * e.wheel.y));
                }
                break;

            case SDL_WINDOWEVENT:
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        set_quit();
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resize(e.window.data1, e.window.data2);
                        break;

                    case SDL_WINDOWEVENT_ENTER:
                        int x, y;
                        if(SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK) {
                            mouse_clicked = true;
                            mouse_x = x;
                            mouse_y = y;
                        }
                        else
                            mouse_clicked = false;
                        break;

                    case SDL_WINDOWEVENT_LEAVE:
                        mouse_clicked = false;
                        break;
                }
                break;

            case SDL_RENDER_TARGETS_RESET:
            case SDL_RENDER_DEVICE_RESET:
                warning("Graphics had a mishap");
                break;
        }
    }
}
