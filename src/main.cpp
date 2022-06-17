#include "parse_cli_args.h"
#include "program.h"
#include "graphics.h"
#include "init_sdl_audio.h"
#include "cache.h"
#include "startup_timer.h"
#include "quit.h"
#include "error.h"

#include "experiments/experiments.h"

#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"
#include "config/cli_args.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fftw3.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <filesystem>  // is_directory, exists
#include <fstream>  // Reading rsc dir verification file
#include <string>

#include <sstream>  // Output formatting
#include <algorithm>  // std::clamp(), std::min(), std::max()

#include <functional>  // std::function


void perform_experiment() {
    try {
        const std::function<void()> experiment_func = str_to_experiment.at(cli_args.experiment_string);
        experiment_func();
    }
    catch(const std::out_of_range &e) {
        error("Incorrect usage; experiment '" + cli_args.experiment_string + "' not known");

        auto it = str_to_experiment.cbegin();
        std::string experiments = it->first;
        for(++it; it != str_to_experiment.cend(); ++it)
            experiments += ", " + it->first;

        hint("Available experiments: " + experiments);
        exit(EXIT_FAILURE);
    }
    catch(const std::exception &e) {
        error("Failed to call experiment function (" + STR(e.what()) + ")");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);  // Redundant, as main also exits after calling perform_experiment()
}


void print_transcription_config() {
    std::stringstream ss;
    ss << "--- Transcription config ---\n";

    ss << "  - Frame size: " << FRAME_SIZE << " samples\n"
       << "  - Frame time: " << ((double)FRAME_SIZE / (double)SAMPLE_RATE) * 1000.0 << " ms\n"
       << "  - Fourier bin size: " << (double)SAMPLE_RATE / (double)FRAME_SIZE << " Hz\n";

    if(ZERO_PAD_FACTOR > 0.0) {
        ss << "  - Frame size with zero padding: " << FRAME_SIZE_PADDED << " samples\n"
           << "  - Interpolated bin size: " << (double)SAMPLE_RATE / (double)FRAME_SIZE_PADDED << " Hz\n";
    }

    if(DO_OVERLAP) {
        const int overlap_n_samples = std::clamp((int)(OVERLAP_RATIO * (double)FRAME_SIZE), 1, FRAME_SIZE - 1);
        ss << "  - Overlap ratio: " << OVERLAP_RATIO << "  (" << overlap_n_samples << " overlapping samples)" << '\n'
           << "  - Frame time without overlap: " << ((double)(FRAME_SIZE - overlap_n_samples) / (double)SAMPLE_RATE) * 1000.0 << " ms\n";
    }

    if(DO_OVERLAP_NONBLOCK) {
        const int min_overlap_n_samples = std::max((int)(MIN_NONBLOCK_OVERLAP_RATIO * (double)FRAME_SIZE), 1);
        const int max_overlap_n_samples = std::min((int)(MAX_NONBLOCK_OVERLAP_RATIO * (double)FRAME_SIZE), FRAME_SIZE - 1);

        ss << "  - Minimum non-blocking overlap ratio: " << MIN_NONBLOCK_OVERLAP_RATIO << "  (" << min_overlap_n_samples << " overlapping samples)\n"
           << "  - Maximum non-blocking overlap ratio: " << MAX_NONBLOCK_OVERLAP_RATIO << "  (" << max_overlap_n_samples << " overlapping samples)\n"
           << "  - Frame time between " << ((MIN_NONBLOCK_OVERLAP_RATIO * FRAME_SIZE) * 1000.0) / (double)SAMPLE_RATE << " and " << ((MAX_NONBLOCK_OVERLAP_RATIO * FRAME_SIZE) * 1000.0) / (double)SAMPLE_RATE << " ms\n";
    }

    // TODO: Better estimate
    // if constexpr(!HEADLESS)
    //     ss << "Max data history RAM usage: " << (double)(((FRAME_SIZE / 2) + 1) * sizeof(double) * MAX_HISTORY_DATAPOINTS) / (double)(1024 * 1024) << " Mb\n"

    info(ss.str());
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
    // catch(const std::exception &e) {
    //     error("Failed to make canonical path from resource path (" + STR(e.what()) + ")");
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

    std::ifstream verification_file(cli_args.rsc_dir + "verify");
    if(!verification_file.is_open()) {
        warning("Failed to open resource directory verification file");
        return false;
    }

    std::string line;
    verification_file >> line;
    if(line != "4c3f666590eeb398f4606555d3756350") {
        error("Resource directory verification failed");
        return false;
    }

    return true;
}


int main(int argc, char *argv[]) {
    // Quitting with ctrl+c in terminal and stacktrace on segfault
    set_signal_handlers();

    parse_args(argc, argv);
    if(!verify_cli_args()) {
        // TODO: Print something? (verify_cli_args() already prints error)
        exit(EXIT_FAILURE);
    }

    if(!verify_rsc_dir()) {
        hint("You have to point to the resource directory if not running from project root using '--rsc <path>'");
        exit(EXIT_FAILURE);
    }

    Cache::init_cache();

    // Can't be done straight from parse_args(), as cache needs to be initialized
    if(cli_args.do_experiment) {
        perform_experiment();
        exit(EXIT_SUCCESS);
    }

    // Init SDL with only audio
    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        error("SDL could not initialize\nSDL Error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // Init audio devices
    const bool playing_back = cli_args.playback || cli_args.synth;
    const bool recording = cli_args.audio_input_method == SampleGetters::audio_in;  // TODO: Condition

    if(playing_back || recording) {
        print_audio_driver();
        __msg("");  // Print newline for clarity
    }

    SDL_AudioDeviceID out_dev;
    if(playing_back) {
        print_playback_devices();
        init_playback_device(out_dev);
    }

    SDL_AudioDeviceID in_dev;
    if(recording) {
        print_recording_devices();
        init_recording_device(in_dev);
    }

    print_transcription_config();

    Graphics *graphics = nullptr;
    if constexpr(!HEADLESS) {
        // Init SDL's font rendering engine
        if(TTF_Init() != 0) {
            error("TTF rendering engine could not initialize\nTTF error: " + STR(TTF_GetError()));
            exit(EXIT_FAILURE);
        }

        if(SDL_InitSubSystem(SDL_INIT_VIDEO) < 0)
            warning("SDL could not initialize video; running headless instead\nSDL Error: " + STR(SDL_GetError()));
        else
            graphics = new Graphics();
    }

    // Init program logic
    Program *program = new Program(graphics, &in_dev, &out_dev);


    // Main program starts now, so init is done
    startup_timer.set_init_time();

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << startup_timer.get_init_time();
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

    // __msg("");  // Clear any partial message
    return EXIT_SUCCESS;
}
