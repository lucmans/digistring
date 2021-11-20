
#include "sample_getter.h"

#include "performance.h"
#include "config.h"
#include "error.h"

#include <SDL2/SDL.h>

#include <chrono>  // timing
#include <thread>  // sleep
#include <cmath>


SampleGetter::SampleGetter(SDL_AudioDeviceID *const _in) {
    in_dev = _in;

    if(settings.generate_sine)
        sound_source = SoundSource::generate_sine;
    else
        sound_source = SoundSource::audio_in;

    generated_wave_freq = settings.generate_sine_freq;
}

SampleGetter::~SampleGetter() {

}


void SampleGetter::add_generated_wave_freq(const double d_freq) {
    generated_wave_freq += d_freq;
}


void SampleGetter::read_frame_float32_audio_device(float *const in, const int n_samples) {
    perf.push_time_point("Start waiting for frame");

    uint32_t read = 0;
    while(read < n_samples * sizeof(float)) {
        uint32_t ret = SDL_DequeueAudio(*in_dev, in + (read / sizeof(float)), (n_samples * sizeof(float)) - read);
        if(ret > (n_samples * sizeof(float)) - read) {
            error("Read too big");
            exit(EXIT_FAILURE);
        }
        else if(ret % 4 != 0) {
            error("Read part of a float32\n");
            exit(EXIT_FAILURE);
        }

        // TODO: IMPORTANT! Calculate lower limit for wait time till enough samples are ready (and sleep this time)
        // Sleep to prevent 100% CPU usage
        if(ret == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        read += ret;
    }

    perf.push_time_point("Read " + STR(n_samples) + " samples");
}

void SampleGetter::read_frame_int32_audio_device(float *const in, const int n_samples) {
    perf.push_time_point("Start waiting for frame");

    uint32_t read = 0;
    int32_t in_buf[n_samples] = {};  // TODO: Remove VLA
    while(read < n_samples * sizeof(int32_t)) {
        uint32_t ret = SDL_DequeueAudio(*in_dev, in_buf, (n_samples * sizeof(int32_t)) - read);
        if(ret > (n_samples * sizeof(int32_t)) - read) {
            error("Read too big");
            exit(EXIT_FAILURE);
        }
        else if(ret % 4 != 0) {
            error("Read part of a int32\n");
            exit(EXIT_FAILURE);
        }

        // TODO: IMPORTANT! Calculate lower limit for wait time till enough samples are ready (and sleep this time)
        // Sleep to prevent 100% CPU usage
        if(ret == 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // // Experiment: Comment this for extra performance
        // if(read + ret == n_samples * sizeof(int32_t))
        //     perf.push_time_point("Read " + STR(n_samples) + " samples");

        // Directly perform conversion to save on computation when frame is ready
        // In other words, the only added latency is from casting the data from the last DequeueAudio call
        for(unsigned int i = 0; i < ret / sizeof(int32_t); i++)
            in[(read / sizeof(int32_t)) + i] = (float)in_buf[i];

        read += ret;
    }

    perf.push_time_point("Read frame and converted from int32 to float32");
}


void SampleGetter::get_frame(float *const in, const int n_samples) {
    switch(sound_source) {
        case SoundSource::audio_in:
            if(AUDIO_FORMAT == AUDIO_F32SYS)
                read_frame_float32_audio_device(in, n_samples);
            else if(AUDIO_FORMAT == AUDIO_S32SYS)
                read_frame_int32_audio_device(in, n_samples);
            // else {  // Caught by static assert in config.h
            //     error("Unknown ");
            //     return;
            // }
            break;

        case SoundSource::generate_sine:
            for(int i = 0; i < n_samples; i++)
                in[i] = sinf((2.0 * M_PI * i * generated_wave_freq) / (float)SAMPLE_RATE);
            break;

        // case SoundSource::file:
        //     if(!file_get_samples(in, n_samples)) {
        //         std::cout << "Finished playing file; quitting after this frame" << std::endl;
        //         set_quit();
        //     }
        //     // SDL_Delay(1000.0 * (double)n_samples / (double)SAMPLE_RATE);  // Real-time file playback
        //     break;
    }
}
