#include "error.h"

#include "config/audio.h"
#include "config/transcription.h"
#include "config/cli_args.h"

#include <SDL2/SDL.h>

#include <sstream>  // Output formatting
#include <iomanip>  // std::setw(), std::setprecision()
#include <algorithm>  // std::max()
#include <numeric>  // std::accumulate()
#include <vector>


// SDL uses 0 or 1 to differentiate between a playback and recording audio device
constexpr int PLAYBACK = 0;
constexpr int RECORDING = 1;


void print_audio_settings(const SDL_AudioSpec &want, const SDL_AudioSpec &have, const bool input) {
    const int precision = 2;
    const double want_latency = ((double)want.samples / (double)want.freq) * 1000.0,
                 have_latency = ((double)have.samples / (double)have.freq) * 1000.0;

    // Table headers
    const std::string sample_rate = "Sample rate",
                      format = "Format",
                      channels = "Channels",
                      samples_per_buffer = "Samples per buffer",
                      buffer_size = "Buffer size",
                      driver_latency = "Driver latency (ms)",
                      silence_value = "Silence value";

    const int spacing = 2;
    const std::vector<size_t> column_width = {
        std::max({
            sample_rate.size(),
            format.size(),
            channels.size(),
            samples_per_buffer.size(),
            buffer_size.size(),
            driver_latency.size(),
            silence_value.size()
        }),
        std::max({
            std::to_string(want.freq).size(),
            std::to_string(want.format).size(),
            std::to_string(want.channels).size(),
            std::to_string(want.samples).size(),
            std::to_string((int)want_latency).size() + 1 + precision,
        }) + spacing,
        std::max({
            std::to_string(have.freq).size(),
            std::to_string(have.format).size(),
            std::to_string(have.channels).size(),
            std::to_string(have.samples).size(),
            std::to_string(have.size).size(),
            std::to_string((int)have_latency).size() + 1 + precision,
            std::to_string(have.silence).size()
        }) + spacing
    };

    // Cast channels and silence to int, as uint8_t is treated as a character
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision)
       << "--- Audio " << (input ? "input" : "output") << " config ---\n"
       << "| " << std::setw(column_width[0]) << "setting"          << std::setw(column_width[1]) << "want"             << std::setw(column_width[2]) << "have"             << " |\n"
       << "|-" << std::string(std::accumulate(column_width.begin(), column_width.end(), 0), '-')                                                                      << "-|\n"
       << "| " << std::setw(column_width[0]) << sample_rate        << std::setw(column_width[1]) << want.freq          << std::setw(column_width[2]) << have.freq          << " |\n"
       << "| " << std::setw(column_width[0]) << format             << std::setw(column_width[1]) << want.format        << std::setw(column_width[2]) << have.format        << " |\n"
       << "| " << std::setw(column_width[0]) << channels           << std::setw(column_width[1]) << (int)want.channels << std::setw(column_width[2]) << (int)have.channels << " |\n"
       << "| " << std::setw(column_width[0]) << samples_per_buffer << std::setw(column_width[1]) << want.samples       << std::setw(column_width[2]) << have.samples       << " |\n"
       << "| " << std::setw(column_width[0]) << buffer_size        << std::setw(column_width[1]) << "-"                << std::setw(column_width[2]) << have.size          << " |\n"
       << "| " << std::setw(column_width[0]) << driver_latency     << std::setw(column_width[1]) << want_latency       << std::setw(column_width[2]) << have_latency       << " |\n"
       << "| " << std::setw(column_width[0]) << silence_value      << std::setw(column_width[1]) << "-"                << std::setw(column_width[2]) << (int)have.silence  << " |\n"
       << std::flush;

    info(ss.str());
}


bool check_change(const SDL_AudioSpec &want, const SDL_AudioSpec &have, const std::string &audio_dev) {
    bool changed = false;

    if(want.freq != have.freq) {
        warning(audio_dev + " sampling rate changed");
        changed = true;
    }

    if(want.format != have.format) {
        warning(audio_dev + " format changed");
        changed = true;
    }

    if(want.channels != have.channels) {
        warning(audio_dev + " number of channels changed");
        changed = true;
    }

    // This only affects driver latency, so change is no problem
    // if(want.samples != have.samples) {
    //     warning(audio_dev + " samples per buffer changed");
    //     changed = true;
    // }

    return changed;
}


void print_audio_driver() {
    const char *const current_driver = SDL_GetCurrentAudioDriver();
    if(current_driver == NULL) {
        error("No audio driver was initialized");
        exit(EXIT_FAILURE);
    }
    info("Using audio driver: " + STR(current_driver));

    // std::stringstream ss;
    // ss << "--- Available audio drivers ---\n";
    // const int count = SDL_GetNumAudioDrivers();
    // for(int i = 0; i < count; i++)
    //     ss << "  * " << SDL_GetAudioDriver(i) << '\n';
    // ss << std::flush;
    // info(ss.str());
}


void print_playback_devices() {
    const int count = SDL_GetNumAudioDevices(PLAYBACK);
    if(count == -1) {
        warning("Failed to get list of playback devices");
        hint("The default playback device can still likely be opened");
    }
    else {
        std::stringstream out_info;
        out_info << "--- Playback devices ---\n";
        out_info << " " << (cli_args.out_dev_name == "" ? "->" : "  ") << " "
                 << "Device 0: System default" << "\n";
        for(int i = 0; i < count; i++) {
            const std::string device_name = SDL_GetAudioDeviceName(i, PLAYBACK);
            out_info << " " << (device_name == cli_args.out_dev_name ? "->" : "  ") << " "
                     << "Device " << i + 1 << ": '" << device_name << "'\n";
        }
        out_info << std::flush;
        info(out_info.str());
    }
}

void print_recording_devices() {
    const int count = SDL_GetNumAudioDevices(RECORDING);
    if(count == -1) {
        warning("Failed to get list of recording devices");
        hint("The default recording device can still likely be opened");
    }
    else {
        std::stringstream in_info;
        in_info << "--- Recording devices ---\n";
        in_info << " " << (cli_args.in_dev_name == "" ? "->" : "  ") << " "
                << "Device 0: System default" << "\n";
        for(int i = 0; i < count; i++) {
            const std::string device_name = SDL_GetAudioDeviceName(i, RECORDING);
            in_info << " " << (device_name == cli_args.in_dev_name ? "->" : "  ") << " "
                    << "Device " << i + 1 << ": '" << device_name << "'\n";
        }
        in_info << std::flush;
        info(in_info.str());
    }
}


void init_playback_device(SDL_AudioDeviceID &out_dev) {
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));  // Because SDL does this on wiki page
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_FORMAT;
    want.channels = (cli_args.stereo_split ? 2 : 1);
    want.samples = SAMPLES_PER_BUFFER;
    want.callback = NULL;

    const char *out_dev_str = (cli_args.out_dev_name == "" ? NULL : cli_args.out_dev_name.c_str());
    out_dev = SDL_OpenAudioDevice(out_dev_str, PLAYBACK, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(out_dev == 0) {
        error("Failed to open audio output\n" + STR(SDL_GetError()));
        hint("Audio settings might not be realistic");
        exit(EXIT_FAILURE);
    }

    if(want.channels != have.channels) {
        error("Failed to get " + STR(want.channels) + " audio channels on playback audio device");
        if(want.channels == 2)
            hint("Try disabling stereo split playback and synthesis");
        exit(EXIT_FAILURE);
    }

    if(check_change(want, have, "Playback device")) {
        if constexpr(ALLOW_PLAYBACK_CHANGE) {
            warning("Letting SDL implicitly convert audio setting changes");
            info("Changes:");
            print_audio_settings(want, have, false);
            SDL_CloseAudioDevice(out_dev);
            out_dev = SDL_OpenAudioDevice(out_dev_str, PLAYBACK, &want, &have, 0);  // Pass 0 for implicit conversion
            if(out_dev == 0) {
                error("Failed to open audio output\n" + STR(SDL_GetError()));
                exit(EXIT_FAILURE);
            }
            return;
        }
        else {
            error("Failed to open playback device with requested audio settings");
            hint("Audio settings can be changed in config/audio.h; or implicit conversion can be enabled");
            info("SDL's suggestion:");
            print_audio_settings(want, have, false);
            exit(EXIT_FAILURE);
        }
    }

    print_audio_settings(want, have, false);
}

void init_recording_device(SDL_AudioDeviceID &in_dev) {
    SDL_AudioSpec want, have;
    SDL_memset(&want, 0, sizeof(want));  // Because SDL does this on wiki page
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_FORMAT;
    want.channels = 1;
    want.samples = SAMPLES_PER_BUFFER;
    want.callback = NULL;

    const char *in_dev_str = (cli_args.in_dev_name == "" ? NULL : cli_args.in_dev_name.c_str());
    in_dev = SDL_OpenAudioDevice(in_dev_str, RECORDING, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if(in_dev == 0) {
        error("Failed to open audio input\n" + STR(SDL_GetError()));
        hint("Audio settings might not be realistic");
        exit(EXIT_FAILURE);
    }

    if(check_change(want, have, "Recording device")) {
        error("Failed to open recording device with requested audio settings");
        hint("Audio settings can be changed in config/audio.h");
        info("SDL's suggestion:");
        print_audio_settings(want, have, true);
        exit(EXIT_FAILURE);
    }

    print_audio_settings(want, have, true);
}
