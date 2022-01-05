
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
}

Program::~Program() {
    delete sample_getter;
}


void Program::main_loop() {
    // We let the estimator create the input buffer for optimal size and better alignment
    int input_buffer_size;
    float *input_buffer = HighRes::create_input_buffer(input_buffer_size);
    Estimator *estimator = new HighRes(input_buffer);

    // Frame limiting
    std::chrono::steady_clock::time_point prev_frame = std::chrono::steady_clock::now();
    std::chrono::duration<double, std::milli> frame_time = std::chrono::duration<double>(0.0);

    // Unpause audio devices so that samples are collected/played
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
            SDL_QueueAudio(*out_dev, input_buffer, input_buffer_size * sizeof(float));

            // Wait till previous frame has played (needed when fetching samples is faster than playing)
            while(SDL_GetQueuedAudioSize(*out_dev) > input_buffer_size * sizeof(float));
        }

        // Send frame to estimator
        NoteSet noteset;
        estimator->perform(input_buffer, noteset);
        perf.push_time_point("Performed estimation");

        // Print note estimation
        std::cout << noteset << "     \r" << std::flush;

        // Graphics
        if constexpr(!HEADLESS) {
            // Get data from estimator for graphics
            graphics->set_max_recorded_value_if_larger(estimator->get_max_norm());

            graphics->add_data_point(estimator->get_spectrum_data());
            perf.push_time_point("Graphics parsed data");

            // Render data
            frame_time = std::chrono::steady_clock::now() - prev_frame;
            if(frame_time.count() > 1000.0 / 15.0) {
                graphics->render_frame();
                perf.push_time_point("Frame rendered");

                prev_frame = std::chrono::steady_clock::now();
            }
        }

        if(settings.output_performance)
            std::cout << perf << std::endl;
    }

    delete estimator;
    Estimator::free_input_buffer(input_buffer);
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
                        // sample_getter->add_generated_wave_freq(-5.0);
                        sample_getter->note_down();
                        break;

                    case SDLK_EQUALS:
                        // sample_getter->add_generated_wave_freq(+5.0);
                        sample_getter->note_up();
                        break;

                    case SDLK_r:
                        graphics->set_max_recorded_value(0.0);
                        break;

                    case SDLK_p:
                        graphics->next_plot_type();
                        break;

                    case SDLK_k:
                        graphics->add_max_display_frequency(-300.0);
                        break;

                    case SDLK_l:
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

            case SDL_WINDOWEVENT:
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        set_quit();
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resize(e.window.data1, e.window.data2);
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
