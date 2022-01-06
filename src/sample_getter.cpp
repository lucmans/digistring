
#include "sample_getter.h"

#include "performance.h"
#include "config.h"
#include "error.h"

#include "note.h"

#include <SDL2/SDL.h>

#include <chrono>  // timing
#include <thread>  // sleep
#include <cmath>


SampleGetter::SampleGetter(SDL_AudioDeviceID *const _in) : note(Notes::A, 4) {
    in_dev = _in;

    if(settings.generate_sine) {
        info("Playing generate_sine");
        sound_source = SoundSource::generate_sine;
    }
    else if(settings.generate_note) {
        info("Playing generate_note");
        sound_source = SoundSource::generate_note;
    }
    else if(settings.play_file) {
        info("Playing play_file");
        sound_source = SoundSource::file;
    }
    else
        sound_source = SoundSource::audio_in;

    generated_wave_freq = settings.generate_sine_freq;
}

SampleGetter::~SampleGetter() {

}


SoundSource SampleGetter::get_sound_source() const {
    return sound_source;
}


void SampleGetter::add_generated_wave_freq(const double d_freq) {
    generated_wave_freq += d_freq;

    std::cout << "Playing sine wave of " << generated_wave_freq << " Hz" << std::endl;
}


void SampleGetter::set_note(const Note &new_note) {
    note = new_note;

    std::cout << "Playing note " << note << "  (" << note.freq << " Hz)" << std::endl;
}

void SampleGetter::note_up() {
    if(note.note == Notes::B)
        note = Note(Notes::C, note.octave + 1);
    else
        note = Note(static_cast<Notes>(static_cast<int>(note.note) + 1), note.octave);

    std::cout << "Playing note " << note << "  (" << note.freq << " Hz)" << std::endl;
}

void SampleGetter::note_down() {
    if(note.note == Notes::C)
        note = Note(Notes::B, note.octave - 1);
    else
        note = Note(static_cast<Notes>(static_cast<int>(note.note) - 1), note.octave);

    std::cout << "Playing note " << note << "  (" << note.freq << " Hz)" << std::endl;
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
        if(ret == 0) {
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

    perf.push_time_point("Read frame and converted from int32 to float32");
}


void SampleGetter::get_frame(float *const in, const int n_samples) {
    static double last_phase = 0.0;

    switch(sound_source) {
        case SoundSource::audio_in:
            if constexpr(AUDIO_FORMAT == AUDIO_F32SYS)
                read_frame_float32_audio_device(in, n_samples);
            else if constexpr(AUDIO_FORMAT == AUDIO_S32SYS)
                read_frame_int32_audio_device(in, n_samples);
            else {  // Caught by static assert in config.h
                error("Unsupported audio format");
                return;
            }
            break;

        case SoundSource::generate_sine:
            for(int i = 0; i < n_samples; i++) {
                const double offset = (last_phase * ((double)SAMPLE_RATE / generated_wave_freq));
                in[i] = sinf((2.0 * M_PI * ((double)i + offset) * generated_wave_freq) / (double)SAMPLE_RATE);
            }
            last_phase = fmod(last_phase + (generated_wave_freq / ((double)SAMPLE_RATE / (double)n_samples)), 1.0);
            break;

        case SoundSource::generate_note:
            for(int i = 0; i < n_samples; i++) {
                const double offset = (last_phase * ((double)SAMPLE_RATE / note.freq));
                in[i] = sinf((2.0 * M_PI * ((double)i + offset) * note.freq) / (double)SAMPLE_RATE);
            }
            last_phase = fmod(last_phase + (note.freq / ((double)SAMPLE_RATE / (double)n_samples)), 1.0);
            break;

        case SoundSource::file:
            std::cout << settings.play_file_name << std::endl;
            error("Not yet implemented!");
            exit(EXIT_FAILURE);
            break;
        //     if(!file_get_samples(in, n_samples)) {
        //         std::cout << "Finished playing file; quitting after this frame" << std::endl;
        //         set_quit();
        //     }
        //     // SDL_Delay(1000.0 * (double)n_samples / (double)SAMPLE_RATE);  // Real-time file playback
        //     break;
    }
}
