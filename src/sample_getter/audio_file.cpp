
#include "audio_file.h"

#include "../config.h"
#include "../error.h"

#include <SDL2/SDL.h>

#include <string>
#include <algorithm>
#include <cstring>  // memcpy(), memset()


AudioFile::AudioFile(const std::string &file) {
    uint8_t *read_buffer;
    uint32_t read_buffer_bytes;

    if(SDL_LoadWAV(file.c_str(), &wav_spec, &read_buffer, &read_buffer_bytes) == NULL) {
        error("Failed to open WAV file '" + file + "'\nSDL error: " + SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if(wav_spec.freq != SAMPLE_RATE) {
        error("Internal sample rate (" + STR(SAMPLE_RATE) + ") mismatches WAV sample rate (" + STR(wav_spec.freq));
        exit(EXIT_FAILURE);
    }

    if(wav_spec.channels != 1) {
        error("Only mono WAVs are supported");
        exit(EXIT_FAILURE);
    }

    if(wav_spec.format == AUDIO_F32SYS) {
        if(read_buffer_bytes % sizeof(float) != 0) {
            error("WAV file '" + file + "' has a non 4 byte float sample");
            exit(EXIT_FAILURE);
        }

        wav_buffer_samples = read_buffer_bytes / sizeof(float);
        wav_buffer = new float[wav_buffer_samples];

        memcpy(wav_buffer, read_buffer, read_buffer_bytes);
    }
    else if(wav_spec.format == AUDIO_S32SYS) {
        if(read_buffer_bytes % sizeof(int32_t) != 0) {
            error("WAV file '" + file + "' has a non 4 byte integer sample");
            exit(EXIT_FAILURE);
        }

        wav_buffer_samples = read_buffer_bytes / sizeof(int32_t);
        wav_buffer = new float[wav_buffer_samples];

        const int32_t *const sample_buffer = (int32_t*)read_buffer;
        for(int i = 0; i < wav_buffer_samples; i++) {
            // | can be interchanged with +
            // const int32_t new_sample = (read_buffer[(i * 4) + 0]    << 0)
            //                             | (read_buffer[(i * 4) + 1] << 8)
            //                             | (read_buffer[(i * 4) + 2] << 16)
            //                             | (read_buffer[(i * 4) + 3] << 24);

            const int32_t new_sample = sample_buffer[i];

            // Volatile, so it doesn't get optimized away
            volatile double d_sample = (double)new_sample / (double)(1 << 31);
            volatile float f_sample = (float)d_sample;
            wav_buffer[i] = f_sample;

            // TODO: Maybe check if new_sample > float mantissa limit
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wfloat-equal"
            static bool shown = false;
            if(!shown && f_sample != d_sample) {
                warning("Lossy conversion of input file");
                shown = true;
            }
            #pragma GCC diagnostic pop
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
    else {
        error("Sample format not supported");
        exit(EXIT_FAILURE);
    }

    SDL_FreeWAV(read_buffer);
}

AudioFile::~AudioFile() {
    delete[] wav_buffer;
}


SampleGetters AudioFile::get_type() const {
    return SampleGetters::audio_file;
}


void AudioFile::get_frame(float *const in, const int n_samples) {
    int overlap_n_samples = n_samples;
    float *overlap_in = in;
    if constexpr(DO_OVERLAP)
        calc_and_paste_overlap(overlap_in, overlap_n_samples);

    // If file end doesn't align with a frame, we need to read less then n_samples
    const int n_read_samples = std::clamp(overlap_n_samples, 0, std::max(wav_buffer_samples - played_samples, 0));
    if(played_samples < wav_buffer_samples)
        memcpy(overlap_in, wav_buffer + played_samples, n_read_samples * sizeof(float));

    // Zero rest of buffer if file ended
    if(n_read_samples < overlap_n_samples)
        memset(overlap_in + n_read_samples, 0, (overlap_n_samples - n_read_samples) * sizeof(float));

    // Check if file has ended
    if(n_read_samples == 0) {
        info("File ended, filling rest of frame with silence...");
        set_quit();  // TODO: Maybe play file till overlap only contains silence
    }

    played_samples += overlap_n_samples;


    if constexpr(DO_OVERLAP)
        copy_overlap(in, n_samples);
}
