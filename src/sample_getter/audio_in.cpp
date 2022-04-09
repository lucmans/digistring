#include "audio_in.h"

#include "performance.h"
#include "error.h"
#include "quit.h"

#include "config/audio.h"
#include "config/transcription.h"

#include <SDL2/SDL.h>

#include <chrono>  // timing
#include <thread>  // sleep
#include <algorithm>  // std::clamp(), std::min()


AudioIn::AudioIn(const int input_buffer_size, SDL_AudioDeviceID *const _in) : SampleGetter(input_buffer_size) {
    in_dev = _in;

    conv_buf = nullptr;
    conv_buf_size = -1;
}

AudioIn::~AudioIn() {
    if(conv_buf != nullptr)
        delete[] conv_buf;
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
        // Ignore VLA warning, as it is debug code anyway
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wvla"
        float ignore[n_samples];
        #pragma GCC diagnostic pop

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

    // Wait till almost enough samples are ready to be retrieved to minimize CPU usage when idle
    const int samples_left = n_samples - (SDL_GetQueuedAudioSize(*in_dev) / sizeof(float));
    const double sample_left_time = ((double)samples_left / (double)SAMPLE_RATE) * 1000.0;  // ms
    // debug("Sleeping for " + STR((int)sample_left_time));
    std::this_thread::sleep_for(std::chrono::microseconds((int)(sample_left_time * SLEEP_FACTOR * 1000.0)));

    uint32_t read = 0;
    while(read < n_samples * sizeof(float)) {
        const uint32_t ret = SDL_DequeueAudio(*in_dev, in + (read / sizeof(float)), (n_samples * sizeof(float)) - read);
        if(ret > (n_samples * sizeof(float)) - read) {
            error("Read too big");
            exit(EXIT_FAILURE);
        }
        else if(ret % 4 != 0) {
            error("Read part of a float32\n");
            exit(EXIT_FAILURE);
        }

        /* Disabled, as this function mostly blocks on sleep at the start of function
        // For quiting during long blocking due to waiting for samples from SDL_DequeueAudio()
        if(poll_quit())
            return;
        */

        read += ret;

        // A full means enough samples were ready to be retrieved, which implies we could've read earlier (less latency)
        if(ret == n_samples * sizeof(float)) {
            warning("Full read; there might still be samples left in input audio queue (latency)");
            hint("Might be due to sleeping too long before first SDL_DequeueAudio()\nTry lowering SLEEP_FACTOR in config/audio.h");
        }
    }

    // DEBUG: For analyzing the range of the float samples, as being within [-1.0, 1.0] is not enforced by any standard
    // static double highest = 0.0, lowest = 0.0;
    // for(int i = 0; i < n_samples; i++) {
    //     if(in[i] < 0.0) {
    //         if(in[i] < lowest)
    //             lowest = in[i];
    //     }
    //     else {
    //         if(in[i] > highest)
    //             highest = in[i];
    //     }
    // }
    // debug(STR(lowest) + " " + STR(highest));

    played_samples += n_samples;
    perf.push_time_point("Read " + STR(n_samples) + " samples");
}

void AudioIn::read_frame_int32_audio_device(float *const in, const int n_samples) {
    perf.push_time_point("Start waiting for frame");

    if(n_samples > conv_buf_size) {
        if(conv_buf != nullptr)
            delete[] conv_buf;

        conv_buf_size = n_samples;
        conv_buf = new int32_t[conv_buf_size];
    }

    // Wait till almost enough samples are ready to be retrieved to minimize CPU usage when idle
    const int samples_left = n_samples - (SDL_GetQueuedAudioSize(*in_dev) / sizeof(uint32_t));
    const double sample_left_time = ((double)samples_left / (double)SAMPLE_RATE) * 1000.0;  // ms
    // debug("Sleeping for " + STR((int)sample_left_time));
    std::this_thread::sleep_for(std::chrono::microseconds((int)(sample_left_time * SLEEP_FACTOR * 1000.0)));

    uint32_t read = 0;
    while(read < n_samples * sizeof(int32_t)) {
        const uint32_t ret = SDL_DequeueAudio(*in_dev, conv_buf, (n_samples * sizeof(int32_t)) - read);
        if(ret > (n_samples * sizeof(int32_t)) - read) {
            error("Read too big");
            exit(EXIT_FAILURE);
        }
        else if(ret % 4 != 0) {
            error("Read part of a int32\n");
            exit(EXIT_FAILURE);
        }

        /* Disabled, as this function mostly blocks on sleep at the start of function
        // For quiting during long blocking due to waiting for samples from SDL_DequeueAudio()
        if(poll_quit())
            return;
        */

        // // Experiment: Comment this for extra performance
        // if(read + ret == n_samples * sizeof(int32_t))
        //     perf.push_time_point("Read " + STR(n_samples) + " samples");

        // Directly perform conversion to save on computation when frame is ready
        // In other words, the only added latency is from casting the data from the last DequeueAudio call
        for(unsigned int i = 0; i < ret / sizeof(int32_t); i++)
            in[(read / sizeof(int32_t)) + i] = (float)conv_buf[i];

        read += ret;

        // A full means enough samples were ready to be retrieved, which implies we could've read earlier (less latency)
        if(ret == n_samples * sizeof(uint32_t)) {
            warning("Full read; implies sleeping too long before first SDL_DequeueAudio()");
            hint("Try lowering SLEEP_FACTOR in config/audio.h");
        }
    }

    played_samples += n_samples;
    perf.push_time_point("Read frame and converted from int32 to float32");
}


void AudioIn::calc_and_paste_nonblocking_overlap(float *&in, int &n_samples, const int bytes_per_sample) {
    // DEBUG
    if(MIN_NEW_SAMPLES_NONBLOCK > n_samples) {
        error("MIN_NEW_SAMPLES_NONBLOCK too large");
        hint("MIN_NEW_SAMPLES_NONBLOCK is set in config/transcription.h");
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
