#include "parse_cli_args.h"
#include "program.h"
#include "graphics.h"
#include "cache.h"
#include "performance.h"
#include "quit.h"
#include "error.h"

#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"
#include "config/cli_args.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fftw3.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <csignal>  // catching ctrl+c in terminal
#include <filesystem>  // is_directory, exists
#include <fstream>  // Reading rsc dir verification file
#include <string>
#include <iostream>

#include <sstream>  // This and iomanip are for double->string formatting
#include <iomanip>


void print_audio_settings(SDL_AudioSpec &specs, bool input) {
    std::cout << "Audio " << (input ? "input" : "output") << " config:\n"
              << "Sample rate: " << SAMPLE_RATE << " " << specs.freq << '\n'
              << "Format: " << AUDIO_FORMAT << " " << specs.format << '\n'
              << "Channels: " << N_CHANNELS << " " << (int)specs.channels << '\n'
              << "Samples per buffer: " << SAMPLES_PER_BUFFER << " " << specs.samples << '\n'
              << "Buffer size:  -  " << specs.size << " bytes\n"
              << "Silence value:  -  " << (int)specs.silence << '\n'
              << std::endl;
}

void print_program_config_info() {
    std::cout << "Frame size: " << FRAME_SIZE << '\n'
              << "Frame size with zero padding: " << FRAME_SIZE_PADDED << '\n'
              << "Frame time: " << ((double)FRAME_SIZE * 1000.0) / (double)SAMPLE_RATE << " ms\n"
              << "Fourier bin size: " << (double)SAMPLE_RATE / (double)FRAME_SIZE << "Hz\n";

    if(DO_OVERLAP)
        std::cout << "Overlap ratio: " << OVERLAP_RATIO << '\n';

    if(DO_OVERLAP_NONBLOCK) {
        std::cout << "Minimum non-blocking frame advance: " << MIN_NEW_SAMPLES_NONBLOCK << " samples  (" << ((double)MIN_NEW_SAMPLES_NONBLOCK * 1000.0) / (double)SAMPLE_RATE << " ms)\n"
                  << "Maximum non-blocking frame advance: " << MAX_NEW_SAMPLES_NONBLOCK << " samples  (" << ((double)MAX_NEW_SAMPLES_NONBLOCK * 1000.0) / (double)SAMPLE_RATE << " ms)\n";
    }

    std::cout << std::endl;

    // TODO: Better estimate
    // if constexpr(!HEADLESS) {
    //     std::cout << "Max data history RAM usage: " << (double)(((FRAME_SIZE / 2) + 1) * sizeof(double) * MAX_HISTORY_DATAPOINTS) / (double)(1024 * 1024) << " Mb\n"
    //               << std::endl;
    // }
}


void print_audio_devices(const bool print_out_dev) {
    int count;

    if(print_out_dev) {
        count = SDL_GetNumAudioDevices(0);
        std::cout << "Playback devices:\n";
        for(int i = 0; i < count; i++)
            std::cout << "  Audio device " << i << ": " << SDL_GetAudioDeviceName(i, 0) << '\n';
        std::cout << std::endl;
    }

    count = SDL_GetNumAudioDevices(1);
    std::cout << "Recording devices:\n";
    for(int i = 0; i < count; i++)
        std::cout << "  Audio device " << i << ": " << SDL_GetAudioDeviceName(i, 0) << '\n';
    std::cout << std::endl;
}

void init_audio_devices(SDL_AudioDeviceID &in_dev, SDL_AudioDeviceID &out_dev, const bool print_out_dev) {
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
    if(print_out_dev)
        print_audio_settings(have, false);

    in_dev = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0/*SDL_AUDIO_ALLOW_ANY_CHANGE*/);
    if(in_dev == 0) {
        error("Failed to open audio input\n" + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    print_audio_settings(have, true);
}


bool verify_rsc_dir() {
    if(!std::filesystem::exists(cli_args.rsc_dir)) {
        error("Resource path doesn't exist");
        return false;
    }

    if(!std::filesystem::is_directory(cli_args.rsc_dir)) {
        error("Resource path is not a directory");
        return false;
    }

    // Clean the path string (for printing it to cli)
    std::filesystem::path path(cli_args.rsc_dir);
    path = path.lexically_normal();

    // Make absolute path
    // try {
    //     path = std::filesystem::canonical(path);
    // }
    // catch(...) {
    //     error("Canonical path '" + std::string(path.string()) + "' doesn't exist");
    //     exit(EXIT_FAILURE);
    // }

    cli_args.rsc_dir = path.string();

    // Make the path end with a /
    if(cli_args.rsc_dir.back() != '/')
        cli_args.rsc_dir += '/';

    if(!std::filesystem::exists(cli_args.rsc_dir + "verify")) {
        error("Resource directory verification file not present");
        return false;
    }

    std::ifstream file(cli_args.rsc_dir + "verify");
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
        hint("You have to point to the resource directory if not running from project root using '--rsc <path>'");
        exit(EXIT_FAILURE);
    }

    Cache::init_cache();

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
    print_audio_devices(cli_args.playback);
    init_audio_devices(in_dev, out_dev, cli_args.playback || cli_args.synth);

    print_program_config_info();

    // Init program logic
    Program *program = new Program(graphics, &in_dev, &out_dev);


    // Main program starts now, so init is done
    perf.set_init_time();

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << perf.get_init_time();
    info("Start-up time: " + ss.str() + " ms");

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
