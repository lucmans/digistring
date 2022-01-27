
#include "audio_in.h"

#include "../performance.h"
#include "../config.h"
#include "../error.h"

#include <SDL2/SDL.h>

#include <chrono>  // timing
#include <thread>  // sleep
#include <algorithm>  // std::clamp(), std::min()


AudioIn::AudioIn(SDL_AudioDeviceID *const _in) {
    in_dev = _in;

    // last_overlap_size = 0;
}

AudioIn::~AudioIn() {

}


SampleGetters AudioIn::get_type() const {
    return SampleGetters::audio_in;
}


// DEBUG
void AudioIn::read_increment(float *const in, const int n_samples) {
    for(int i = 0; i < n_samples; i++)
        in[i] = played_samples + i + 1;

    played_samples += n_samples;

    // wait as if performing a read
    uint32_t read = 0;
    while(read < n_samples * sizeof(float)) {
        float ignore[n_samples];
        uint32_t ret = SDL_DequeueAudio(*in_dev, ignore, (n_samples * sizeof(float)) - read);
        if(ret > (n_samples * sizeof(float)) - read) {
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


void AudioIn::read_frame_float32_audio_device(float *const in, const int n_samples) {
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
        if(ret == 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        read += ret;
    }

    played_samples += n_samples;
    perf.push_time_point("Read " + STR(n_samples) + " samples");
}

void AudioIn::read_frame_int32_audio_device(float *const in, const int n_samples) {
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
        if(ret == 0) {
            // std::cout << "Sleeping" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            // const int samples_left = n_samples - (read / sizeof(int32_t));
            // const int sample_left_time = ((double)samples_left / (double)SAMPLE_RATE) * 1000.0;  // ms
            // std::this_thread::sleep_for(std::chrono::milliseconds(sample_left_time - 1));
        }

        // // Experiment: Comment this for extra performance
        // if(read + ret == n_samples * sizeof(int32_t))
        //     perf.push_time_point("Read " + STR(n_samples) + " samples");

        // Directly perform conversion to save on computation when frame is ready
        // In other words, the only added latency is from casting the data from the last DequeueAudio call
        for(unsigned int i = 0; i < ret / sizeof(int32_t); i++)
            in[(read / sizeof(int32_t)) + i] = (float)in_buf[i];

        read += ret;
    }

    played_samples += n_samples;
    perf.push_time_point("Read frame and converted from int32 to float32");
}


void AudioIn::calc_and_paste_nonblocking_overlap(float *&in, int &n_samples, const int bytes_per_sample) {
    // DEBUG
    if(MIN_NEW_SAMPLES_NONBLOCK > n_samples) {
        debug("MIN_NEW_SAMPLES_NONBLOCK too large");
        exit(EXIT_FAILURE);
    }

    const int samples_queued = SDL_GetQueuedAudioSize(*in_dev) / bytes_per_sample;

    // Clamp so at least one sample is overlapped or kept between frames
    const int n_overlap = std::clamp(n_samples - samples_queued, n_samples - MAX_NEW_SAMPLES_NONBLOCK, n_samples - MIN_NEW_SAMPLES_NONBLOCK);

    // Paste the overlap to start of 'in'
    memcpy(in, overlap_buffer + (n_samples - n_overlap), n_overlap * sizeof(float));

    // Calculate the remainder of the buffer
    in += n_overlap;
    n_samples -= n_overlap;
}

void AudioIn::copy_nonblocking_overlap(float *const in, const int n_samples) {
    // TODO: Copy less
    memcpy(overlap_buffer, in, n_samples * sizeof(float));
}


void AudioIn::get_frame(float *const in, const int n_samples) {
    int overlap_n_samples = n_samples;
    float *overlap_in = in;
    if constexpr(DO_OVERLAP)
        calc_and_paste_overlap(overlap_in, overlap_n_samples);
    else if constexpr(DO_OVERLAP_NONBLOCK)
        calc_and_paste_nonblocking_overlap(overlap_in, overlap_n_samples, SDL_AUDIO_BITSIZE(AUDIO_FORMAT) / 8);

    // read_increment(overlap_in, overlap_n_samples);
    if constexpr(AUDIO_FORMAT == AUDIO_F32SYS)
        read_frame_float32_audio_device(overlap_in, overlap_n_samples);
    else if constexpr(AUDIO_FORMAT == AUDIO_S32SYS)
        read_frame_int32_audio_device(overlap_in, overlap_n_samples);
    else {  // Caught by static assert in config.h
        error("Unsupported audio format");
        exit(EXIT_FAILURE);
    }


    if constexpr(DO_OVERLAP)
        copy_overlap(in, n_samples);
    else if constexpr(DO_OVERLAP_NONBLOCK)
        copy_nonblocking_overlap(in, n_samples);
}
