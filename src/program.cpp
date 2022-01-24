
#include "program.h"

#include "graphics.h"
#include "performance.h"
#include "config.h"
#include "error.h"

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
    // int input_buffer_n_samples;
    estimator = new HighRes(input_buffer, input_buffer_n_samples);
    if(input_buffer_n_samples > MAX_FRAME_SIZE) {
        error("Read buffer is larger than maximum frame size");
        exit(EXIT_FAILURE);
    }

    if(settings.generate_sine) {
        // info("Playing generate_sine");
        sample_getter = new WaveGenerator(settings.generate_sine_freq);
    }
    else if(settings.generate_note) {
        // info("Playing generate_note");
        sample_getter = new NoteGenerator(settings.generate_note_note);
    }
    else if(settings.play_file) {
        // info("Playing '" + settings.play_file_name + "'");
        sample_getter = new AudioFile(settings.play_file_name);
    }
    else {
        // info("Sourcing audio from computer audio input");
        sample_getter = new AudioIn(in_dev);
    }

    mouse_clicked = false;

    if(settings.output_file) {
        output_stream.open(settings.output_filename, std::fstream::out);
        if(!output_stream.is_open()) {
            error("Failed to create/open file '" + settings.output_filename + "'");
            exit(EXIT_FAILURE);
        }

        output_stream << "Sample rate: " << SAMPLE_RATE << '\n'
                      << "Frame size: " << FRAME_SIZE << '\n'
                      << "Frame time: " << ((double)FRAME_SIZE * 1000.0) / (double)SAMPLE_RATE << " ms\n"
                      << "Fourier bin size: " << (double)SAMPLE_RATE / (double)FRAME_SIZE << " Hz"
                      << std::endl;
    }

    lag = 0;

    note_change_time = std::chrono::duration<double>(NOTE_TIME + 1);
}

Program::~Program() {
    output_stream.close();

    delete sample_getter;

    delete estimator;
}


void Program::main_loop() {
    // Unpause audio devices so that samples are collected/played
    if(!(settings.generate_sine || settings.generate_note || settings.play_file))
        SDL_PauseAudioDevice(*in_dev, 0);

    if(settings.playback)
        SDL_PauseAudioDevice(*out_dev, 0);

    if(settings.output_file)
        output_stream << "\nplaytime (s), note, frequency (Hz), amplitude (dB), error (cent)" << std::endl;

    while(!poll_quit()) {
        perf.clear_time_points();
        perf.push_time_point("Start");

        // Check for keyboard/mouse/window/OS events
        handle_sdl_events();

        // DEBUG
        if(lag != 0) {  // Lag might have been increased by in handle_sdl_events()
            debug("Lagging for " + STR(lag) + " ms");
            std::this_thread::sleep_for(std::chrono::milliseconds(lag));
            lag = 0;
        }

        perf.push_time_point("Handled SDL events");

        // Read a frame
        sample_getter->get_frame(input_buffer, input_buffer_n_samples);
        perf.push_time_point("Got frame full of audio samples");

        // Play it back to the user if chosen
        if(settings.playback)
            playback_audio();

        // Send frame to estimator
        NoteSet noteset;
        estimator->perform(input_buffer, noteset);
        perf.push_time_point("Performed estimation");

        // Print note estimation
        // print_results(noteset);

        // Write estimation to output file
        if(settings.output_file)
            write_results(noteset);

        // Graphics
        if constexpr(!HEADLESS)
            update_graphics(noteset);

        // Print performance information to CLI
        if(settings.output_performance)
            std::cout << perf << std::endl;

        // Arpeggiator easter egg
        if constexpr(ENABLE_ARPEGGIATOR)
            arpeggiate();
    }
}


void Program::resize(const int w, const int h) {
    graphics->resize_window(w, h);
}


void Program::playback_audio() {
    if constexpr(DISPLAY_AUDIO_UNDERRUNS)
        if(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) == 0)
            warning("Audio underrun; no audio left to play");

    if(SDL_QueueAudio(*out_dev, input_buffer, input_buffer_n_samples * sizeof(float))) {
        error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // DEBUG: If the while() below works correctly, the out buffer should never be filled faster than it is played
    const bool playing_audio_in = !(settings.generate_sine || settings.generate_note || settings.play_file);
    if(playing_audio_in && SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) > (unsigned int)input_buffer_n_samples * 1.9) {
        warning("Audio overrun (too much audio to play); clearing buffer...");
        SDL_ClearQueuedAudio(*out_dev);
    }

    // Wait till one frame is left in systems audio out buffer (needed when fetching samples is faster than playing)
    while(SDL_GetQueuedAudioSize(*out_dev) / (SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8) >= (unsigned int)input_buffer_n_samples && !poll_quit())
        handle_sdl_events();
}


void Program::write_results(const NoteSet &noteset) {
    const int n_notes = noteset.size();
    if(n_notes == 0)
        return;

    else if(n_notes == 1) {
        const std::string note = note_to_string(noteset[0]);
        output_stream << std::setw(12) << std::fixed << std::setprecision(4);
        output_stream << sample_getter->get_played_time() - ((double)input_buffer_n_samples / (double)SAMPLE_RATE) / 2.0 << ", " << (note.size() == 4 ? " " : "") << note << ", " << noteset[0].freq << ", " << noteset[0].amp << ",  " << noteset[0].error << std::endl;
    }

    else  // n_notes > 1
        warning("Polyphonic output is not yet supported");  // TODO: Support
}

void Program::print_results(const NoteSet &noteset) {
    if(noteset.size() > 0)
        std::cout << noteset[0] << "  (" << noteset[0].freq << " Hz, " << noteset[0].amp << " dB, " << noteset[0].error << " cent)" << "     \r" << std::flush;
}


void Program::update_graphics(const NoteSet &noteset) {
    // Get data from estimator for graphics
    graphics->set_max_recorded_value_if_larger(estimator->get_max_norm());

    const Spectrum *spec = estimator->get_spectrum();
    graphics->add_data_point(spec->get_data());
    perf.push_time_point("Graphics parsed data");

    // Render data
    frame_time = std::chrono::steady_clock::now() - prev_frame;
    if(frame_time.count() > 1000.0 / MAX_FPS) {
        graphics->set_clicked((mouse_clicked ? mouse_x : -1), mouse_y);

        const int n_notes = noteset.size();
        if(n_notes == 0)
            graphics->render_frame(nullptr);
        else if(n_notes == 1)
            graphics->render_frame(&noteset[0]);
        else  // n_notes > 1
            warning("Polyphonic graphics not yet supported");  // TODO: Support

        perf.push_time_point("Frame rendered");

        prev_frame = std::chrono::steady_clock::now();
    }
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
                        graphics->set_max_recorded_value(0.0);
                        break;

                    case SDLK_t:
                        SDL_ClearQueuedAudio(*out_dev);
                        break;

                    case SDLK_p:
                        graphics->next_plot_type();
                        break;

                    case SDLK_LEFTBRACKET:
                        graphics->add_max_display_frequency(-300.0);
                        break;

                    case SDLK_RIGHTBRACKET:
                        graphics->add_max_display_frequency(300.0);
                        break;

                    case SDLK_f:
                        graphics->toggle_freeze_graph();
                        break;

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
