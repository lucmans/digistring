
#include "program.h"

#include "window_func.h"
#include "graphics.h"
#include "performance.h"
#include "config.h"
#include "error.h"

#include <fftw3.h>

#include <cmath>


Program::Program(Graphics *const _g, SDL_AudioDeviceID *const _in, SDL_AudioDeviceID *const _out) {
    graphics = _g;

    in_dev = _in;
    out_dev = _out;

    in = (float*)fftwf_malloc(FRAME_SIZE * sizeof(float));
    out = (fftwf_complex*)fftwf_malloc(((FRAME_SIZE / 2) + 1) * sizeof(fftwf_complex));
    p = fftwf_plan_dft_r2c_1d(FRAME_SIZE, in, out, FFTW_ESTIMATE);

    // Pre-calculate window function
    blackman_nuttall_window(window_func);

    if(settings.generate_sine)
        sound_source = SoundSource::generate_sine;
    else
        sound_source = SoundSource::audio_in;

    generated_wave_freq = settings.generate_sine_freq;
}

Program::~Program() {
    fftwf_free(in);
    fftwf_free(out);
    fftwf_destroy_plan(p);
}


void Program::main_loop() {
    // Unpause audio devices so that samples are collected/played
    SDL_PauseAudioDevice(*in_dev, 0);
    if(settings.playback)
        SDL_PauseAudioDevice(*out_dev, 0);

    while(!poll_quit()) {
        perf.clear_time_points();
        perf.push_time_point("Start");

        handle_sdl_events();
        perf.push_time_point("Handled SDL events");

        transcribe();
        perf.push_time_point("Transcribed frame");


        if(!settings.headless) {
            graphics->render_frame();
            perf.push_time_point("Frame rendered");
        }

        if(settings.output_performance)
            std::cout << perf << std::endl;
    }
}


void Program::resize(const int w, const int h) {
    graphics->resize_window(w, h);
}



void Program::handle_sdl_events() {
    SDL_Event e;
    while(SDL_PollEvent(&e) && !poll_quit()) {
        switch(e.type) {
            case SDL_QUIT:
            case SDL_APP_TERMINATING:
                set_quit();
                break;

            case SDL_KEYDOWN:
                switch(e.key.keysym.sym) {
                    case SDLK_q:
                    case SDLK_ESCAPE:
                        set_quit();
                        break;

                    case SDLK_MINUS:
                        generated_wave_freq -= 5.0;
                        break;

                    case SDLK_EQUALS:
                        generated_wave_freq += 5.0;
                        break;

                    // TODO: Switching sound source and on switching, call SDL_ClearQueuedAudio(*in_dev)

                    // // sleep
                    // case SDLK_s:  // DEBUG
                    //     std::this_thread::sleep_for(std::chrono::seconds(1));
                    //     break;

                }
                break;

            case SDL_WINDOWEVENT:
                switch(e.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                        set_quit();
                        break;

                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        resize(e.window.data1, e.window.data2);
                        break;
                }
                break;

            case SDL_RENDER_TARGETS_RESET:
            case SDL_RENDER_DEVICE_RESET:
                warning("Graphics had a mishap");
                break;
        }
    }
}


void Program::read_frame_audio_in(float *const in) {
    uint32_t read = 0;
    while(read < FRAME_SIZE * sizeof(float)) {
        uint32_t ret = SDL_DequeueAudio(*in_dev, in + (read / sizeof(float)), (FRAME_SIZE * sizeof(float)) - read);
        if(ret > (FRAME_SIZE * sizeof(float)) - read) {
            error("Read too big");
            exit(EXIT_FAILURE);
        }
        else if(ret % 4 != 0) {
            error("Read part of a float\n");
            exit(EXIT_FAILURE);
        }

        read += ret;
    }
}

void Program::get_frame(float *const in) {
    switch(sound_source) {
        case SoundSource::audio_in:
            read_frame_audio_in(in);
            break;

        case SoundSource::generate_sine:
            for(int i = 0; i < FRAME_SIZE; i++)
                in[i] = sinf((2.0 * M_PI * i * generated_wave_freq) / (float)SAMPLE_RATE);
            break;

        // case SoundSource::file:
        //     if(!file_get_samples(in, FRAME_SIZE)) {
        //         std::cout << "Finished playing file; quitting after this frame" << std::endl;
        //         set_quit();
        //     }
        //     // SDL_Delay(1000.0 * (double)FRAME_SIZE / (double)SAMPLE_RATE);  // Real-time file playback
        //     break;
    }
}


// Normalize results: http://fftw.org/fftw3_doc/The-1d-Discrete-Fourier-Transform-_0028DFT_0029.html
void calc_norms(const fftwf_complex values[], double norms[(FRAME_SIZE / 2) + 1]) {
    for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++)
        norms[i] = sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1]));
}

void calc_norms(const fftwf_complex values[], double norms[(FRAME_SIZE / 2) + 1], double &max_norm, double &power) {
    max_norm = -1.0;
    power = 0.0;

    for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++) {
        norms[i] = sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1]));
        power += norms[i];

        if(norms[i] > max_norm)
            max_norm = norms[i];
    }
}

// dB ref: https://www.kvraudio.com/forum/viewtopic.php?t=276092
void calc_norms_db(const fftwf_complex values[], double norms[(FRAME_SIZE / 2) + 1]) {
    for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++)
        norms[i] = 20 * log10(sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
}

void calc_norms_db(const fftwf_complex values[], double norms[(FRAME_SIZE / 2) + 1], double &max_norm, double &power) {
    max_norm = -1.0;
    power = 0.0;

    for(int i = 1; i < (FRAME_SIZE / 2) + 1; i++) {
        norms[i] = 20 * log10(sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
        power += norms[i];

        if(norms[i] > max_norm)
            max_norm = norms[i];
    }
}


// TODO: Replace old code with new
void Program::transcribe() {
    // Start reading a window
    // This will block until enough samples have been generated by capturing audio device
    get_frame(in);
    perf.push_time_point("Read frame");
    if(settings.playback)
        SDL_QueueAudio(*out_dev, in, FRAME_SIZE * sizeof(float));

    // Apply window function to minimize spectral leakage
    for(int i = 0; i < FRAME_SIZE; i++)
        in[i] *= (float)window_func[i];  // TODO: Have float versions of the window functions
    perf.push_time_point("Applied window function");

    // Do the actual transform
    fftwf_execute(p);
    perf.push_time_point("Fourier transformed");

    double norms[(FRAME_SIZE / 2) + 1] = {};
    double power, max_norm;
    calc_norms(out, norms, max_norm, power);
    perf.push_time_point("Norms calculated");

    if(!settings.headless) {
        graphics->set_max_recorded_value_if_larger(max_norm);
        graphics->add_data_point(norms);
    }
}
