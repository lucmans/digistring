#include "audio_file.h"

#include "quit.h"
#include "error.h"

#include "config/audio.h"
#include "config/transcription.h"

#include <SDL2/SDL.h>

#include <string>
#include <algorithm>  // std::max(), std::clamp(), std::fill_n()
#include <cstring>  // memcpy()
#include <limits>

#include <sstream>  // This and iomanip are for double->string formatting
#include <iomanip>


AudioFile::AudioFile(const int input_buffer_size, const std::string &file) : SampleGetter(input_buffer_size) {
    uint8_t *read_buffer;
    uint32_t read_buffer_bytes;

    if(SDL_LoadWAV(file.c_str(), &wav_spec, &read_buffer, &read_buffer_bytes) == NULL) {
        error("Failed to open WAV file '" + file + "'\nSDL error: " + SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if(wav_spec.freq != SAMPLE_RATE) {
        error("Internal sample rate (" + STR(SAMPLE_RATE) + " Hz) mismatches WAV sample rate (" + STR(wav_spec.freq) + " Hz)");
        hint("Internal sample rate can be set in src/config/audio.h");
        exit(EXIT_FAILURE);
    }

    if(wav_spec.channels != 1) {
        error("Only mono WAVs are supported");
        exit(EXIT_FAILURE);
    }

    if(wav_spec.format == AUDIO_F32SYS) {
        if(sizeof(float) != 4) {
            error("Floats are not 32 bits on this platform; which is a problem when directly interfacing with sample formats");
            hint("Add conversion to 32 bit floats, like is done with integers");
            exit(EXIT_FAILURE);
        }

        if(read_buffer_bytes % sizeof(float) != 0) {
            error("WAV file '" + file + "' has a non 32 bit float sample");
            exit(EXIT_FAILURE);
        }

        wav_buffer_n_samples = read_buffer_bytes / sizeof(float);
        try {
            wav_buffer = new float[wav_buffer_n_samples];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to allocate WAV file buffer; WAV file too big (" + STR(e.what()) + ")");
            hint("Try a splitting the WAV file into smaller files");
            exit(EXIT_FAILURE);
        }

        memcpy(wav_buffer, read_buffer, read_buffer_bytes);
    }
    else if(wav_spec.format == AUDIO_S32SYS) {
        if(read_buffer_bytes % sizeof(int32_t) != 0) {
            error("WAV file '" + file + "' has a non 4 byte integer sample");
            exit(EXIT_FAILURE);
        }

        wav_buffer_n_samples = read_buffer_bytes / sizeof(int32_t);
        try {
            wav_buffer = new float[wav_buffer_n_samples];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to allocate WAV file buffer; WAV file too big (" + STR(e.what()) + ")");
            hint("Try a splitting the WAV file into smaller files");
            exit(EXIT_FAILURE);
        }

        const int32_t *const sample_buffer = (int32_t*)read_buffer;

        // Check if samples are 24-bit or 32-bit
        bool bit32 = false;
        for(int i = 0; i < wav_buffer_n_samples; i++) {
            if((sample_buffer[i] & 0b11111111) != 0b00000000)
                bit32 = true;
        }
        if(bit32)
            warning("File uses 32 bit samples, lossless conversion");

        for(int i = 0; i < wav_buffer_n_samples; i++) {
            // | can be interchanged with +
            // const int32_t new_sample = (read_buffer[(i * 4) + 0]    << 0)
            //                             | (read_buffer[(i * 4) + 1] << 8)
            //                             | (read_buffer[(i * 4) + 2] << 16)
            //                             | (read_buffer[(i * 4) + 3] << 24);

            const int32_t new_sample = sample_buffer[i];

            if(bit32) {
                const double d_sample = (double)new_sample / (double)(std::numeric_limits<int32_t>::max());
                wav_buffer[i] = (float)d_sample;
            }
            else {
                // SDL makes 8 least significant bits zero when converting from 24 bit files
                const double d_sample = (double)(new_sample >> 8) / (double)((1 << 23) - 1);
                wav_buffer[i] = (float)d_sample;
            }
        }

        // SDL based conversion
        /*SDL_AudioCVT cvt;
        SDL_BuildAudioCVT(&cvt, AUDIO_S32SYS, 1, SAMPLE_RATE, AUDIO_F32SYS, 1, SAMPLE_RATE);
        SDL_assert(cvt.needed);
        cvt.len = read_buffer_bytes;
        cvt.buf = (Uint8*)SDL_malloc(cvt.len * cvt.len_mult);
        memcpy(cvt.buf, read_buffer, read_buffer_bytes);
        SDL_ConvertAudio(&cvt);*/
    }
    else if(wav_spec.format == AUDIO_S16SYS) {
        if(read_buffer_bytes % sizeof(int16_t) != 0) {
            error("WAV file '" + file + "' has a non 2 byte integer sample");
            exit(EXIT_FAILURE);
        }

        wav_buffer_n_samples = read_buffer_bytes / sizeof(int16_t);
        try {
            wav_buffer = new float[wav_buffer_n_samples];
        }
        catch(const std::bad_alloc &e) {
            error("Failed to allocate WAV file buffer; WAV file too big (" + STR(e.what()) + ")");
            hint("Try a splitting the WAV file into smaller files");
            exit(EXIT_FAILURE);
        }

        const int16_t *const sample_buffer = (int16_t*)read_buffer;
        for(int i = 0; i < wav_buffer_n_samples; i++) {
            const int16_t new_sample = sample_buffer[i];

            const double d_sample = (double)new_sample / (double)(std::numeric_limits<int16_t>::max());
            wav_buffer[i] = (float)d_sample;
        }

        // SDL based conversion
        /*SDL_AudioCVT cvt;
        SDL_BuildAudioCVT(&cvt, AUDIO_S16SYS, 1, SAMPLE_RATE, AUDIO_F32SYS, 1, SAMPLE_RATE);
        SDL_assert(cvt.needed);
        cvt.len = read_buffer_bytes;
        cvt.buf = (Uint8*)SDL_malloc(cvt.len * cvt.len_mult);
        memcpy(cvt.buf, read_buffer, read_buffer_bytes);
        SDL_ConvertAudio(&cvt);*/
    }
    else {
        error("Sample format not supported");
        exit(EXIT_FAILURE);
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << (double)wav_buffer_n_samples / (double)SAMPLE_RATE;
    info("WAV file loaded, " + ss.str() + " seconds long");

    // debug("File is " + STR((double)wav_buffer_n_samples / (double)SAMPLE_RATE) + " seconds; " + STR(wav_buffer_n_samples) + " samples");

    // DEBUG: For analyzing the range of the float samples, as being within [-1.0, 1.0] is not enforced by any standard
    // double highest = 0.0, lowest = 0.0;
    // for(int i = 0; i < wav_buffer_n_samples; i++) {
    //     if(wav_buffer[i] < 0.0) {
    //         if(wav_buffer[i] < lowest)
    //             lowest = wav_buffer[i];
    //     }
    //     else {
    //         if(wav_buffer[i] > highest)
    //             highest = wav_buffer[i];
    //     }
    // }
    // debug(STR(lowest) + " " + STR(highest));

    SDL_FreeWAV(read_buffer);
}

AudioFile::~AudioFile() {
    delete[] wav_buffer;
}


SampleGetters AudioFile::get_type() const {
    return SampleGetters::audio_file;
}


void AudioFile::seek(const int d_samples) {
    played_samples += d_samples;

    // If seeked behind start
    if(played_samples <= 0) {
        played_samples = 0;

        if constexpr(DO_OVERLAP)
            std::fill_n(overlap_buffer, overlap_buffer_size, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles

        // debug("Seeked to " + STR((double)played_samples / (double)SAMPLE_RATE) + " seconds; " + STR(played_samples) + " samples");
        return;
    }

    if(played_samples > wav_buffer_n_samples) {
        info("File ended after seek");
        set_quit();
        return;
    }

    // Fill overlap buffer with correct part of the file
    if constexpr(DO_OVERLAP) {
        // Copy as much before played_samples into overlap_buffer to enable seeking while overlapping
        const int samples_needed_before_file = overlap_buffer_size - played_samples;

        if(samples_needed_before_file > 0) {
            // Zero start of buffer
            std::fill_n(overlap_buffer, samples_needed_before_file, 0.0);

            // Copy as much from file as possible
            memcpy(overlap_buffer + samples_needed_before_file, wav_buffer, played_samples * sizeof(float));
        }
        else
            // Copy as much from file as possible
            memcpy(overlap_buffer, wav_buffer + played_samples - overlap_buffer_size, overlap_buffer_size * sizeof(float));
    }

    // debug("Seeked to " + STR((double)played_samples / (double)SAMPLE_RATE) + " seconds; " + STR(played_samples) + " samples");
}


int AudioFile::get_frame(float *const in, const int n_samples) {
    int overlap_n_samples = n_samples;  // n_samples to get after accounting for overlap
    float *overlap_in = in;
    if constexpr(DO_OVERLAP)
        calc_and_paste_overlap(overlap_in, overlap_n_samples);

    // debug("At to " + STR((double)played_samples / (double)SAMPLE_RATE) + " seconds; " + STR(played_samples) + " samples");

    // If file end doesn't align with a frame, we need to read less than n_samples
    // First calculate how many samples we can still read from the file
    const long file_samples_left = std::max(wav_buffer_n_samples - played_samples, (long)0);  // max(), as played_samples > wav_buffer_n_samples may happen when overlapping
    const int n_samples_from_file = std::clamp((long)overlap_n_samples, (long)0, file_samples_left);  // overlap_n_samples always fits in int, so n_samples_from_file never overflows
    if(played_samples < wav_buffer_n_samples)
        memcpy(overlap_in, wav_buffer + played_samples, n_samples_from_file * sizeof(float));

    // Zero rest of buffer if file ended
    if(n_samples_from_file < overlap_n_samples)
        std::fill_n(overlap_in + n_samples_from_file, overlap_n_samples - n_samples_from_file, 0.0);

    played_samples += overlap_n_samples;

    // Check if there won't be any of the file in the next read and quit if so
    if(played_samples >= wav_buffer_n_samples + (n_samples - overlap_n_samples)) {
        info("File ended, filling rest of frame with silence...");
        set_quit();
    }


    if constexpr(DO_OVERLAP)
        copy_overlap(in, n_samples);

    return overlap_n_samples;
}
