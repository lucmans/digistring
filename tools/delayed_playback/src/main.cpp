#include "parse_cli_args.h"
#include "init_sdl_audio.h"

#include "graphics.h"
#include "config.h"
#include "error.h"
#include "quit.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <algorithm>  // std::fill_n()


volatile static int n_buffers_latency = 0;


void read_buffer(const SDL_AudioDeviceID &in_dev, float *const buffer, const int size) {
    uint32_t read = 0;
    while(read < size * sizeof(float)) {
        const uint32_t ret = SDL_DequeueAudio(in_dev, buffer, (size * sizeof(float)) - read);
        if(ret > (size * sizeof(float)) - read) {
            error("Read too big");
            exit(EXIT_FAILURE);
        }
        else if(ret % 4 != 0) {
            error("Read part of a float32\n");
            exit(EXIT_FAILURE);
        }

        read += ret;
    }
}

void play_buffer(const SDL_AudioDeviceID &out_dev, const float *const buffer, const int size) {
    if(SDL_QueueAudio(out_dev, buffer, size * sizeof(float)) != 0) {
        error("Failed to queue audio\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
}


void handle_input(const SDL_AudioDeviceID &in_dev, const SDL_AudioDeviceID &out_dev, float *const buffer, const int size) {
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

                    case SDLK_EQUALS:
                    case SDLK_KP_PLUS:
                        n_buffers_latency += D_BUFFERS_PER_ACTION;

                        std::fill_n(buffer, size, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles
                        for(int i = 0; i < D_BUFFERS_PER_ACTION; i++)
                            play_buffer(out_dev, buffer, size);
                        break;

                    case SDLK_MINUS:
                    case SDLK_KP_MINUS:
                        if(n_buffers_latency == 0)
                            continue;
                        n_buffers_latency -= D_BUFFERS_PER_ACTION;

                        for(int i = 0; i < D_BUFFERS_PER_ACTION; i++)
                            read_buffer(in_dev, buffer, size);
                        break;

                    // Reset n_buffers_latency
                    case SDLK_r:
                        n_buffers_latency = 0;
                        SDL_ClearQueuedAudio(out_dev);
                        SDL_ClearQueuedAudio(in_dev);
                        break;
                }
                break;

            // case SDL_MOUSEWHEEL:
            //     if(e.wheel.y > 0)
            //         // Increase latency
            //     else if(e.wheel.y < 0)
            //         // Decrease latency
            //     break;

            case SDL_WINDOWEVENT:
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        set_quit();
                        break;
                }
                break;
        }
    }
}


int main(int argc, char *argv[]) {
    set_signal_handlers();
    parse_args(argc, argv);

    if(SDL_Init(SDL_INIT_AUDIO/* | SDL_INIT_VIDEO*/) < 0) {
        error("SDL could not initialize\nSDL Error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    if(TTF_Init() != 0) {
        error("TTF rendering engine could not initialize\nTTF error: " + STR(TTF_GetError()));
        exit(EXIT_FAILURE);
    }

    // Init audio devices
    print_audio_driver();
    __msg("");  // Print newline for clarity

    SDL_AudioDeviceID out_dev;
    print_playback_devices();
    const int playback_samples_per_buffer = init_playback_device(out_dev);

    SDL_AudioDeviceID in_dev;
    print_recording_devices();
    const int recording_samples_per_buffer = init_recording_device(in_dev);

    if(playback_samples_per_buffer != recording_samples_per_buffer) {
        error("Failed to get same number of samples per buffer for input and output");
        debug("TODO: Allow SDL to convert samples per buffer");
        exit(EXIT_FAILURE);
    }
    const int samples_per_buffer = recording_samples_per_buffer;

    float *monitor_buffer;
    try {
        monitor_buffer = new float[samples_per_buffer];
    }
    catch(const std::bad_alloc &e) {
        error("Failed to allocate monitoring buffer");
        exit(EXIT_FAILURE);
    }

    const double driver_latency = ((double)samples_per_buffer * 1000.0) / (double)SAMPLE_RATE;
    Graphics *graphics = new Graphics(driver_latency);


    // Main loop
    SDL_PauseAudioDevice(out_dev, 0);
    SDL_PauseAudioDevice(in_dev, 0);
    while(!poll_quit()) {
        handle_input(in_dev, out_dev, monitor_buffer, samples_per_buffer);

        read_buffer(in_dev, monitor_buffer, samples_per_buffer);
        play_buffer(out_dev, monitor_buffer, samples_per_buffer);

        const double current_latency = (double)(n_buffers_latency * samples_per_buffer * 1000) / (double)SAMPLE_RATE;  // ms
        graphics->render_frame(current_latency);
    }


    // Clean-up
    delete graphics;

    delete[] monitor_buffer;

    SDL_CloseAudioDevice(in_dev);
    SDL_CloseAudioDevice(out_dev);

    TTF_Quit();
    SDL_Quit();

    return EXIT_SUCCESS;
}
