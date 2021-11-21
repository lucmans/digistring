
#include "program.h"
#include "graphics.h"
#include "performance.h"
#include "config.h"
#include "error.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <csignal>  // catching ctrl+c in terminal
#include <cstring>  // strcmp()
#include <string>  // std::stoi()


void main_loop() {

}


void parse_args(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-f") == 0) {
            settings.fullscreen = true;
        }
        // else if(strcmp(argv[i], "-hl") == 0) {
        //     settings.headless = true;
        // }
        else if(strcmp(argv[i], "-p") == 0) {
            settings.playback = true;
        }
        else if(strcmp(argv[i], "-perf") == 0) {
            settings.output_performance = true;
        }
        else if(strcmp(argv[i], "-r") == 0 && argc > i + 2) {
            int n = -1;
            try {
                n = std::stoi(argv[i + 1]);
            }
            catch(...) {
                error("Failed to parse given width '" + std::string(argv[i + 1]) + "'");
                exit(EXIT_FAILURE);
            }
            if(n < MIN_RES[0]) {
                error("Width is too small (" + STR(n) + " < " + STR(MIN_RES[0]) + ")");
                exit(EXIT_FAILURE);
            }
            settings.w = n;

            try {
                n = std::stoi(argv[i + 2]);
            }
            catch(...) {
                error("Failed to parse given height '" + std::string(argv[i + 2]) + "'");
                exit(EXIT_FAILURE);
            }
            if(n < MIN_RES[1]) {
                error("Height is too small (" + STR(n) + " < " + STR(MIN_RES[1]) + ")");
                exit(EXIT_FAILURE);
            }
            settings.h = n;

            i += 2;
        }
        else if(strcmp(argv[i], "-s") == 0) {
            settings.generate_sine = true;

            if(argc > i + 1) {
                if(argv[i + 1][0] == '-')
                    continue;

                double f;
                try {
                    f = std::stod(argv[i + 1]);
                }
                catch(...) {
                    error("Failed to parse frequency '" + std::string(argv[i + 1]) + "'");
                    exit(EXIT_FAILURE);
                }
                if(f < 1) {
                    error("Frequency below 1 Hz (" + STR(f) + ")");
                    exit(EXIT_FAILURE);
                }
                settings.generate_sine_freq = f;

                i++;  // Advance extra argument
            }
        }
        else {
            if(strcmp(argv[i], "-h") != 0 && strcmp(argv[i], "--help") != 0)
                std::cout << "Incorrect usage.\n" << std::endl;

            std::cout << "Flags:\n"
                      << "  -f          - Start in fullscreen. Also set the fullscreen resolution with '-r'\n"
                      // << "  -hl         - Run headless (no window with graphics)\n"
                      << "  -p          - Play recorded audio back\n"
                      << "  -perf       - Output performance stats in stdout\n"
                      << "  -r <w> <h>  - Start GUI with given resolution\n"
                      << "  -s [f]      - Generate sine wave with optional frequency f (default is 1000 Hz) instead of using recording device\n"
                      << std::endl;
            exit(EXIT_SUCCESS);
        }
    }
}


void print_audio_settings(SDL_AudioSpec &specs, bool input) {
    std::cout << "Audio " << (input ? "input" : "output") << " config:" << std::endl
              << "Sample rate: " << SAMPLE_RATE << " " << specs.freq << std::endl
              << "Format: " << AUDIO_FORMAT << " " << specs.format << std::endl
              << "Channels: " << N_CHANNELS << " " << (int)specs.channels << std::endl
              << "Samples per buffer: " << SAMPLES_PER_BUFFER << " " << specs.samples << std::endl
              << "Buffer size:  -  " << specs.size << " bytes" << std::endl
              << "Silence value:  -  " << (int)specs.silence << std::endl
              << std::endl;
}

void print_program_config_info() {
    std::cout << "Frame size: " << FRAME_SIZE << std::endl
              << "Frame time: " << ((double)FRAME_SIZE * 1000.0) / (double)SAMPLE_RATE << " ms" << std::endl
              << "Fourier bin size: " << (double)SAMPLE_RATE / (double)FRAME_SIZE << "Hz" << std::endl
              << "Maximum Fourier frequency: " << MAX_FOURIER_FREQUENCY << " Hz" << std::endl
              << std::endl;

    if constexpr(!HEADLESS) {
        std::cout << "Data history RAM size: " << (double)(((FRAME_SIZE / 2) + 1) * sizeof(double) * MAX_HISTORY_DATAPOINTS) / (double)(1024 * 1024) << " Mb" << std::endl
                  << std::endl;
    }
}


void print_audio_devices() {
    int count = SDL_GetNumAudioDevices(0);
    std::cout << "Playback devices:" << std::endl;
    for(int i = 0; i < count; i++)
        std::cout << "  Audio device " << i << ": " << SDL_GetAudioDeviceName(i, 0) << std::endl;
    std::cout << std::endl;

    count = SDL_GetNumAudioDevices(1);
    std::cout << "Recording devices:" << std::endl;
    for(int i = 0; i < count; i++)
        std::cout << "  Audio device " << i << ": " << SDL_GetAudioDeviceName(i, 0) << std::endl;
    std::cout << std::endl;
}

void init_audio_devices(SDL_AudioDeviceID &in_dev, SDL_AudioDeviceID &out_dev) {
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_FORMAT;
    want.channels = N_CHANNELS;
    want.samples = SAMPLES_PER_BUFFER;
    want.callback = NULL;

    out_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(out_dev == 0) {
        error("Failed to open audio output\n" + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    print_audio_settings(have, false);

    in_dev = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0/*SDL_AUDIO_ALLOW_ANY_CHANGE*/);
    if(in_dev == 0) {
        error("Failed to open audio input\n" + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    print_audio_settings(have, true);
}


int main(int argc, char *argv[]) {
    // Init program
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    reset_quit();

    parse_args(argc, argv);

    // Init SDL
    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        error("SDL could not initialize\nSDL Error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    info("Using audio driver: " + STR(SDL_GetCurrentAudioDriver()));

    // Init SDL's font rendering engine
    if(TTF_Init() != 0) {
        error("TTF rendering engine could not initialize\nTTF error: " + STR(TTF_GetError()));
        exit(EXIT_FAILURE);
    }

    Graphics *graphics = nullptr;
    if constexpr(!HEADLESS) {
        if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
            warning("SDL could not initialize video; running headless instead\nSDL Error: " + STR(SDL_GetError()));
        else
            graphics = new Graphics();
    }

    SDL_AudioDeviceID in_dev, out_dev;
    print_audio_devices();
    init_audio_devices(in_dev, out_dev);

    print_program_config_info();

    // Disable SDL key/mouse events to minimize event overhead
    SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_DISABLE);
    SDL_EventState(SDL_MOUSEBUTTONUP, SDL_DISABLE);
    SDL_EventState(SDL_MOUSEMOTION, SDL_DISABLE);
    SDL_EventState(SDL_MOUSEWHEEL, SDL_DISABLE);
    // SDL_EventState(SDL_KEYDOWN, SDL_DISABLE);
    SDL_EventState(SDL_KEYUP, SDL_DISABLE);

    // Init program logic
    Program *program = new Program(graphics, &in_dev, &out_dev);


    // Main program starts now, so init is done
    perf.set_init_time();
    info("Start-up time: " + STR(perf.get_init_time()) + " ms");

    program->main_loop();


    if(graphics != nullptr)
        delete graphics;
    if(program != nullptr)
        delete program;

    SDL_CloseAudioDevice(in_dev);
    SDL_CloseAudioDevice(out_dev);

    TTF_Quit();
    SDL_Quit();

    std::cout << std::endl;
    return EXIT_SUCCESS;
}
