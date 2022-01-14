
#include "program.h"

#include "graphics.h"
#include "sample_getter.h"
#include "performance.h"
#include "config.h"
#include "error.h"

#include "estimators/estimators.h"

#include <SDL2/SDL.h>


Program::Program(Graphics *const _g, SDL_AudioDeviceID *const _in, SDL_AudioDeviceID *const _out) : graphics(_g) {
    // graphics = _g;

    in_dev = _in;
    out_dev = _out;

    sample_getter = new SampleGetter(in_dev);

    mouse_clicked = false;
}

Program::~Program() {
    delete sample_getter;
}


// #include <thread>  // sleep
void Program::main_loop() {
    // We let the estimator create the input buffer for optimal size and better alignment
    float *input_buffer = NULL;
    int input_buffer_size;
    Estimator *estimator = new HighRes(input_buffer, input_buffer_size);

    // Frame limiting
    std::chrono::duration<double, std::milli> frame_time;  // = std::chrono::duration<double>(0.0);
    std::chrono::steady_clock::time_point prev_frame = std::chrono::steady_clock::now();

    // Unpause audio devices so that samples are collected/played
    if(!(settings.generate_sine || settings.generate_note || settings.play_file))
        SDL_PauseAudioDevice(*in_dev, 0);

    if(settings.playback)
        SDL_PauseAudioDevice(*out_dev, 0);

    while(!poll_quit()) {
        perf.clear_time_points();
        perf.push_time_point("Start");

        handle_sdl_events();
        perf.push_time_point("Handled SDL events");

        // Read a frame
        sample_getter->get_frame(input_buffer, input_buffer_size);
        perf.push_time_point("Got frame full of audio samples");

        // Play it back to the user if chosen
        if(settings.playback) {
            if(SDL_QueueAudio(*out_dev, input_buffer, input_buffer_size * sizeof(float))) {
                error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
                exit(EXIT_FAILURE);
            }

            // Wait till previous frame has played (needed when fetching samples is faster than playing)
            while(SDL_GetQueuedAudioSize(*out_dev) > input_buffer_size * sizeof(float) && !poll_quit())
                handle_sdl_events();
        }

        // Send frame to estimator
        NoteSet noteset;
        estimator->perform(input_buffer, noteset);
        perf.push_time_point("Performed estimation");

        // Print note estimation
        // if(noteset.size() > 0)
        //     std::cout << noteset[0] << "  (" << noteset[0].freq << " Hz, " << noteset[0].amp << " dB, " << noteset[0].error << " cent)" << "     \r" << std::flush;

        // Graphics
        if constexpr(!HEADLESS) {
            // Get data from estimator for graphics
            graphics->set_max_recorded_value_if_larger(estimator->get_max_norm());

            const Spectrum *spec = estimator->get_spectrum();
            graphics->add_data_point(spec->get_data());
            perf.push_time_point("Graphics parsed data");

            // Render data
            frame_time = std::chrono::steady_clock::now() - prev_frame;
            if(frame_time.count() > 1000.0 / 15.0) {
                graphics->set_clicked((mouse_clicked ? mouse_x : -1), mouse_y);

                if(noteset.size() > 0)
                    graphics->render_frame(&noteset[0]);
                else
                    graphics->render_frame(nullptr);

                perf.push_time_point("Frame rendered");

                prev_frame = std::chrono::steady_clock::now();
            }
        }

        if(settings.output_performance)
            std::cout << perf << std::endl;
    }

    delete estimator;
}


void Program::resize(const int w, const int h) {
    graphics->resize_window(w, h);
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
                        {
                            SoundSource source = sample_getter->get_sound_source();
                            if(source == SoundSource::generate_sine)
                                sample_getter->add_generated_wave_freq(-5.0);
                            else if(source == SoundSource::generate_note)
                                sample_getter->note_down();
                        }
                        break;

                    case SDLK_EQUALS:
                        {
                            SoundSource source = sample_getter->get_sound_source();
                            if(source == SoundSource::generate_sine)
                                sample_getter->add_generated_wave_freq(+5.0);
                            else if(source == SoundSource::generate_note)
                                sample_getter->note_up();
                        }
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

                    // TODO: Switching sound source, and on switching call SDL_ClearQueuedAudio(*in_dev)

                    // // sleep
                    // case SDLK_s:  // DEBUG
                    //     std::this_thread::sleep_for(std::chrono::seconds(1));
                    //     break;

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
                        // debug((SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK ? "yes" : "no"));
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
