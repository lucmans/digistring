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
#include <filesystem>  // is_directory, exists
#include <fstream>  // Reading rsc dir verification file
#include <string>
#include <iostream>

#include <sstream>  // Output formatting
#include <iomanip>  // std::setw()
#include <algorithm>  // std::max()
#include <numeric>  // std::accumulate()


void print_audio_settings(SDL_AudioSpec &specs, bool input) {
    const std::string sample_rate = "Sample rate",
                      format = "Format",
                      channels = "Channels",
                      samples_per_buffer = "Samples per buffer",
                      buffer_size = "Buffer size",
                      silence_value = "Silence value";

    const std::vector<size_t> column_width = {
        std::max({
            sample_rate.size(),
            format.size(),
            channels.size(),
            samples_per_buffer.size(),
            buffer_size.size(),
            silence_value.size()
        }),
        std::max({
            std::to_string(SAMPLE_RATE).size(),
            std::to_string(AUDIO_FORMAT).size(),
            std::to_string(N_CHANNELS).size(),
            std::to_string(SAMPLES_PER_BUFFER).size()
        }) + 2,
        std::max({
            std::to_string(specs.freq).size(),
            std::to_string(specs.format).size(),
            std::to_string(specs.channels).size(),
            std::to_string(specs.samples).size(),
            std::to_string(specs.size).size(),
            std::to_string(specs.silence).size()
        }) + 2
    };

    std::stringstream ss;
    ss << "--- Audio " << (input ? "input" : "output") << " config ---\n"
       << "| " << std::setw(column_width[0]) << "setting"          << std::setw(column_width[1]) << "want"             << std::setw(column_width[2]) << "have"              << " |\n"
       << "|-" << std::string(std::accumulate(column_width.begin(), column_width.end(), 0), '-')                                                                            << "-|\n"
       << "| " << std::setw(column_width[0]) << sample_rate        << std::setw(column_width[1]) << SAMPLE_RATE        << std::setw(column_width[2]) << specs.freq          << " |\n"
       << "| " << std::setw(column_width[0]) << format             << std::setw(column_width[1]) << AUDIO_FORMAT       << std::setw(column_width[2]) << specs.format        << " |\n"
       << "| " << std::setw(column_width[0]) << channels           << std::setw(column_width[1]) << N_CHANNELS         << std::setw(column_width[2]) << (int)specs.channels << " |\n"
       << "| " << std::setw(column_width[0]) << samples_per_buffer << std::setw(column_width[1]) << SAMPLES_PER_BUFFER << std::setw(column_width[2]) << specs.samples       << " |\n"
       << "| " << std::setw(column_width[0]) << buffer_size        << std::setw(column_width[1]) << "-"                << std::setw(column_width[2]) << specs.size          << " |\n"
       << "| " << std::setw(column_width[0]) << silence_value      << std::setw(column_width[1]) << "-"                << std::setw(column_width[2]) << (int)specs.silence  << " |\n"
       << std::flush;

    info(ss.str());
}

void print_program_config_info() {
    std::stringstream ss;

    ss << "--- Transcription config ---\n";

    ss << "  - Frame size: " << FRAME_SIZE << '\n'
       << "  - Frame size with zero padding: " << FRAME_SIZE_PADDED << '\n'
       << "  - Frame time: " << ((double)FRAME_SIZE / (double)SAMPLE_RATE) * 1000.0 << " ms\n"
       << "  - Fourier bin size: " << (double)SAMPLE_RATE / (double)FRAME_SIZE << "Hz\n";

    if(DO_OVERLAP) {
        const int overlap_n_samples = std::max((int)(OVERLAP_RATIO * (double)FRAME_SIZE), 1);
        ss << "  - Overlap ratio: " << OVERLAP_RATIO << "  (" << overlap_n_samples << " overlapping samples)" << '\n'
           << "  - Frame time without overlap: " << ((double)(FRAME_SIZE - overlap_n_samples) / (double)SAMPLE_RATE) * 1000.0 << " ms\n";
    }

    if(DO_OVERLAP_NONBLOCK) {
        ss << "  - Minimum non-blocking frame advance: " << MIN_NEW_SAMPLES_NONBLOCK << " samples  (" << ((double)MIN_NEW_SAMPLES_NONBLOCK * 1000.0) / (double)SAMPLE_RATE << " ms)\n"
           << "  - Maximum non-blocking frame advance: " << MAX_NEW_SAMPLES_NONBLOCK << " samples  (" << ((double)MAX_NEW_SAMPLES_NONBLOCK * 1000.0) / (double)SAMPLE_RATE << " ms)\n";
    }

    // TODO: Better estimate
    // if constexpr(!HEADLESS)
    //     ss << "Max data history RAM usage: " << (double)(((FRAME_SIZE / 2) + 1) * sizeof(double) * MAX_HISTORY_DATAPOINTS) / (double)(1024 * 1024) << " Mb\n"

    ss << std::flush;
    info(ss.str());
}


void print_audio_driver() {
    info("Using audio driver: " + STR(SDL_GetCurrentAudioDriver()));

    // std::stringstream ss;
    // ss << "--- Available audio drivers ---\n";
    // const int count = SDL_GetNumAudioDrivers();
    // for(int i = 0; i < count; i++)
    //     ss << "  * " << SDL_GetAudioDriver(i) << '\n';
    // ss << std::flush;
    // info(ss.str());
}


void print_audio_devices(const bool print_out_dev) {
    __msg("");  // Print newline for clarity

    if(print_out_dev) {
        const int count = SDL_GetNumAudioDevices(0);
        if(count > 1 && cli_args.out_dev_name == "") {
            warning("SDL is choosing the \"most reasonable\" default playback device");
            hint("If audio playback is not working, try explicitly setting the playback device using '--audio_out <device name>' flag");
        }

        if(count == -1)
            warning("Failed to get list of playback devices");
        else {
            std::stringstream out_info;
            out_info << "--- Playback devices ---\n";
            for(int i = 0; i < count; i++) {
                const std::string device_name = SDL_GetAudioDeviceName(i, 0);
                out_info << " " << (device_name == cli_args.out_dev_name ? "->" : "  ") << " "
                         << "Device " << i + 1 << ": '" << device_name << "'\n";
            }
            out_info << std::flush;
            info(out_info.str());
        }
    }

    const int count = SDL_GetNumAudioDevices(1);
    if(count > 1 && cli_args.in_dev_name == "") {
        warning("SDL is choosing the \"most reasonable\" default recording device");
        hint("If audio recording is not working, try explicitly setting the recording device using '--audio_in <device name>' flag");
    }

    if(count == -1)
        warning("Failed to get list of recording devices");
    else {
        std::stringstream in_info;
        in_info << "--- Recording devices ---\n";
        for(int i = 0; i < count; i++) {
            const std::string device_name = SDL_GetAudioDeviceName(i, 1);
            in_info << " " << (device_name == cli_args.in_dev_name ? "->" : "  ") << " "
                    << "Device " << i + 1 << ": '" << device_name << "'\n";
        }
        in_info << std::flush;
        info(in_info.str());
    }
}

void init_audio_devices(SDL_AudioDeviceID &in_dev, SDL_AudioDeviceID &out_dev, const bool print_out_dev) {
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_FORMAT;
    want.channels = N_CHANNELS;
    want.samples = SAMPLES_PER_BUFFER;
    want.callback = NULL;

    if(cli_args.out_dev_name == "")
        out_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    else
        out_dev = SDL_OpenAudioDevice(cli_args.out_dev_name.c_str(), 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(out_dev == 0) {
        error("Failed to open audio output\n" + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    if(print_out_dev)
        print_audio_settings(have, false);

    if(cli_args.in_dev_name == "")
        in_dev = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0/*SDL_AUDIO_ALLOW_ANY_CHANGE*/);
    else
        in_dev = SDL_OpenAudioDevice(cli_args.in_dev_name.c_str(), 1, &want, &have, 0/*SDL_AUDIO_ALLOW_ANY_CHANGE*/);
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
    // Quitting with ctrl+c in terminal and stacktrace on segfault
    set_signal_handlers();

    parse_args(argc, argv);

    if(!verify_rsc_dir()) {
        hint("You have to point to the resource directory if not running from project root using '--rsc <path>'");
        exit(EXIT_FAILURE);
    }

    Cache::init_cache();

    // Init SDL with only audio
    if(SDL_Init(SDL_INIT_AUDIO) < 0) {
        error("SDL could not initialize\nSDL Error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // Init audio devices
    SDL_AudioDeviceID in_dev, out_dev;
    print_audio_driver();
    print_audio_devices(cli_args.playback || cli_args.synth);
    init_audio_devices(in_dev, out_dev, cli_args.playback || cli_args.synth);

    print_program_config_info();  // Audio and transcription settings

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

    // __msg("");  // Clear any partial message
    return EXIT_SUCCESS;
}
