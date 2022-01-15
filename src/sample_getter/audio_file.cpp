
#include "audio_file.h"

#include "../config.h"
#include "../error.h"

#include <SDL2/SDL.h>

#include <string>
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
        for(unsigned long i = 0; i < wav_buffer_samples; i++) {
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

            static bool shown = false;
            if(!shown && f_sample != d_sample) {
                warning("Lossy conversion of input file");
                shown = true;
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
    static bool ended = false;
    if(ended) {
        // TODO: Quit or listen from audio in
        memset(in, 0, n_samples * sizeof(float));
        return;
    }

    // If file end doesn't align with a frame, we need to read less then n_samples
    const int n_write_samples = std::min(n_samples, (int)(wav_buffer_samples - played_samples));
    memcpy(in, wav_buffer + played_samples, n_write_samples * sizeof(float));

    // Zero rest of buffer if file ended
    if(n_write_samples < n_samples) {
        memset(in + n_write_samples, 0, (n_samples - n_write_samples) * sizeof(float));
        ended = true;

        warning("File ended, continuing with silence...");
    }

    played_samples += n_write_samples;
}
