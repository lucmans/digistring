
#include "parse_args.h"
#include "program.h"
#include "graphics.h"
#include "performance.h"
#include "config.h"
#include "error.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fftw3.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <csignal>  // catching ctrl+c in terminal
#include <filesystem>  // is_directory, exists
#include <fstream>  // Reading rsc dir verification file
#include <string>
#include <iostream>


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
              << "Minimum frame advance (" << (DO_OVERLAP_NONBLOCK ? "on" : "off") << "): " << MIN_OVERLAP_ADVANCE << " samples  (" << ((double)MIN_OVERLAP_ADVANCE * 1000.0) / (double)SAMPLE_RATE << " ms)" << std::endl
              << std::endl;

    if constexpr(!HEADLESS) {
        std::cout << "Max data history RAM usage: " << (double)(((FRAME_SIZE / 2) + 1) * sizeof(double) * MAX_HISTORY_DATAPOINTS) / (double)(1024 * 1024) << " Mb" << std::endl
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


bool verify_rsc_dir() {
    if(!std::filesystem::exists(settings.rsc_dir)) {
        error("Resource path not found");
        return false;
    }

    if(!std::filesystem::is_directory(settings.rsc_dir)) {
        error("Resource path is not a directory");
        return false;
    }

    if(!std::filesystem::exists(settings.rsc_dir + "/verify")) {
        error("Recourse directory verification file not present");
        return false;
    }

    std::ifstream file(settings.rsc_dir + "/verify");
    std::string line;
    file >> line;
    if(line != "4c3f666590eeb398f4606555d3756350") {
        error("Resource directory verification failed");
        file.close();
        return false;
    }
    file.close();

    return true;
}


int main(int argc, char *argv[]) {
    // Init program
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    reset_quit();

    parse_args(argc, argv);

    if(!verify_rsc_dir()) {
        info("You have to point to the resource directory if not running from project root using the '-rsc <path>' flag.");
        exit(EXIT_FAILURE);
    }

    // Init SDL
    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        error("SDL could not initialize\nSDL Error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

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

    info("Using audio driver: " + STR(SDL_GetCurrentAudioDriver()));
    SDL_AudioDeviceID in_dev, out_dev;
    print_audio_devices();
    init_audio_devices(in_dev, out_dev);

    print_program_config_info();

    // Disable SDL key/mouse events to minimize event overhead
    SDL_EventState(SDL_MOUSEWHEEL, SDL_DISABLE);
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

    fftwf_cleanup();

    std::cout << std::endl;
    return EXIT_SUCCESS;
}
