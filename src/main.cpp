
#include "graphics.h"
#include "program.h"
#include "performance.h"
#include "config.h"
#include "error.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <csignal>  // catching ctrl+c in terminal
#include <cstring>  // strcmp()
#include <string>  // std::stoi()


void parse_args(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-f") == 0) {
            settings.fullscreen = true;
        }
        else if(strcmp(argv[i], "-hl") == 0) {
            settings.headless = true;
        }
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
        }
        else {
            if(strcmp(argv[i], "-h") != 0 && strcmp(argv[i], "--help") != 0)
                std::cout << "Incorrect usage.\n" << std::endl;

            std::cout << "Flags:\n"
                      << "  -f          - Start in fullscreen. Also set the fullscreen resolution with '-r'\n"
                      << "  -hl         - Run headless (no window with graphics)\n"
                      << "  -p          - Play recorded audio back\n"
                      << "  -perf       - Output performance stats in stdout\n"
                      << "  -r <w> <h>  - Start GUI with given resolution\n"
                      << "  -s          - Generate sine wave instead of using recording device\n"
                      << std::endl;
            exit(EXIT_SUCCESS);
        }
    }
}


void print_audio_settings(SDL_AudioSpec &specs, bool input) {
    std::cout << "Audio " << (input ? "input" : "output") << " config:" << std::endl
              << "Sample rate: " << SAMPLE_RATE << " " << specs.freq << std::endl
              << "Format: " << FORMAT << " " << specs.format << std::endl
              << "Channels: " << N_CHANNELS << " " << (int)specs.channels << std::endl
              << "Samples per buffer: " << SAMPLES_PER_BUFFER << " " << specs.samples << std::endl
              << "Buffer size:  -  " << specs.size << " bytes" << std::endl
              << "Silence value:  -  " << (int)specs.silence << std::endl
              << std::endl;
}

void print_config() {
    std::cout << "Frame size: " << FRAME_SIZE << std::endl
              << "Frame time: " << (FRAME_SIZE * 1000) / (double)SAMPLE_RATE << " ms" << std::endl
              << "Fourier bin size: " << SAMPLE_RATE / (double)FRAME_SIZE << "Hz" << std::endl
              << std::endl;
}


int main(int argc, char *argv[]) {
    // Init program
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    reset_quit();

    parse_args(argc, argv);

    // Init SDL
    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        error("SDL could not initialize\nSDL Error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    info("Using audio driver: " + STR(SDL_GetCurrentAudioDriver()));

    Graphics *graphics = nullptr;
    if(!settings.headless) {
        if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
            warning("SDL could not initialize video; running headless instead\nSDL Error: " + STR(SDL_GetError()));
        else
            graphics = new Graphics();
    }

    // Init audio devices
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

    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = FORMAT;
    want.channels = N_CHANNELS;
    want.samples = SAMPLES_PER_BUFFER;
    want.callback = NULL;

    SDL_AudioDeviceID out_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0/*SDL_AUDIO_ALLOW_ANY_CHANGE*/);
    if(out_dev == 0) {
        error("Failed to open audio output\n" + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    print_audio_settings(have, false);

    SDL_AudioDeviceID in_dev = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0/*SDL_AUDIO_ALLOW_ANY_CHANGE*/);
    if(in_dev == 0) {
        error("Failed to open audio input\n" + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    print_audio_settings(have, true);

    print_config();

    // Init font rendering engine
    if(TTF_Init() != 0) {
        error("TTF rendering engine could not initialize\nTTF error: " + STR(TTF_GetError()));
        exit(EXIT_FAILURE);
    }

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
