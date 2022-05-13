#include "tuned.h"

#include "performance.h"
#include "note.h"
#include "error.h"

#include "estimation_func.h"
#include "window_func.h"

#include "config/audio.h"
#include "config/transcription.h"
#include "config/graphics.h"

#include <fftw3.h>

#include <cmath>


inline constexpr int fourier_size(const Note &note) {
    return (int)round((double)SAMPLE_RATE / note.freq);
}


Tuned::Tuned(float *&input_buffer, int &buffer_size) {
    constexpr int n_samples = fourier_size(LOWEST_NOTE);

    input_buffer = (float*)fftwf_malloc(n_samples * sizeof(float));
    if(input_buffer == NULL) {
        error("Failed to malloc sample input buffer");
        exit(EXIT_FAILURE);
    }
    buffer_size = n_samples;

    // Calculate the sizes of the transform buffers
    for(int i = 0; i < 12; i++)
        buffer_sizes[i] = fourier_size(Note(LOWEST_NOTE.midi_number + i));

    // Sanity check
    if(buffer_size != buffer_sizes[0]) {
        error("Input buffer size is not equal to the buffer size of the lowest note");
        exit(EXIT_FAILURE);
    }

    // Create transform buffers
    ins[0] = input_buffer;  // Share the sample input buffer with the Fourier input buffer
    for(int i = 1; i < 12; i++) {
        ins[i] = (float*)fftwf_malloc(buffer_sizes[i] * sizeof(float));
        if(ins[i] == NULL) {
            error("Failed to malloc " + STR(i) + "-th Fourier input buffer");
            exit(EXIT_FAILURE);
        }
    }

    // Create the output buffers
    for(int i = 0; i < 12; i++) {
        outs[i] = (fftwf_complex*)fftwf_malloc((((buffer_sizes[i]) / 2) + 1) * sizeof(fftwf_complex));
        if(outs[i] == NULL) {
            error("Failed to malloc " + STR(i) + "-th Fourier output buffer");
            exit(EXIT_FAILURE);
        }
    }

    // Create the planners which actually perform the Fourier transform
    // TODO: Exhaustive planner and graphics to show "optimizing planner"
    for(int i = 0; i < 12; i++)
        plans[i] = fftwf_plan_dft_r2c_1d(buffer_sizes[i], ins[i], outs[i], FFTW_ESTIMATE);


    // Pre-calculate window functions
    for(int i = 0; i < 12; i++) {
        window_funcs[i] = (float*)malloc(buffer_sizes[i] * sizeof(float));
        if(window_funcs[i] == NULL) {
            error("Failed to allocate buffer for window function");
            exit(EXIT_FAILURE);
        }

        blackman_nuttall_window(window_funcs[i], buffer_sizes[i]);
    }

    // Allocate norms here once instead of VLA in perform()
    try {
        norms = new double[(n_samples / 2) + 1];
    }
    catch(const std::bad_alloc &e) {
        error("Failed to allocate norms buffer (" + STR(e.what()) + ")");
        exit(EXIT_FAILURE);
    }

    if constexpr(!HEADLESS)
        estimator_graphics = new TunedGraphics();
}

Tuned::~Tuned() {
    if constexpr(!HEADLESS)
        delete estimator_graphics;

    for(int i = 0; i < 12; i++)
        fftwf_destroy_plan(plans[i]);

    // i = 0 is the input buffer
    for(int i = 0; i < 12; i++)
        fftwf_free(ins[i]);

    for(int i = 0; i < 12; i++)
        fftwf_free(outs[i]);

    for(int i = 0; i < 12; i++)
        free(window_funcs[i]);

    delete[] norms;
}


Estimators Tuned::get_type() const {
    return Estimators::tuned;
}


void Tuned::perform(float *const input_buffer, NoteEvents &note_events) {
    // Note that ins[0] = input_buffer
    double max_norm = 0.0;

    // Copy input to each array
    for(int i = 1; i < 12; i++)
        memcpy(ins[i], input_buffer + (buffer_sizes[0] - buffer_sizes[i]), buffer_sizes[i] * sizeof(float));
    perf.push_time_point("Copied input buffer over");

    // Apply window functions to minimize spectral leakage
    for(int i = 0; i < 12; i++)
        for(int j = 0; j < buffer_sizes[j]; j++)
            ins[i][j] *= window_funcs[i][j];
    perf.push_time_point("Applied window functions");

    // Do the actual transform
    for(int i = 0; i < 12; i++)
        fftwf_execute(plans[i]);  // TODO: OMP multi threading
    perf.push_time_point("Fourier transforms performed");

    // Calculate the amplitudes of each measured frequency
    TunedGraphics *tuned_graphics = nullptr;  // Assign nullptr to prevent warning (it's only set and used when not in headless mode)
    if constexpr(!HEADLESS) {
        tuned_graphics = static_cast<TunedGraphics *>(estimator_graphics);
        tuned_graphics->get_spectrum().clear();
        tuned_graphics->get_note_channel_data().clear();
    }
    int max_power_channel_idx = 0;
    double max_power = -1.0;
    for(int i = 0; i < 12; i++) {
        // double norms[(buffer_sizes[i] / 2) + 1];
        double power;
        double tmp_max_norm = 0.0;
        calc_norms(outs[i], norms, (buffer_sizes[i] / 2) + 1, tmp_max_norm, power);

        if(power > max_power) {
            max_power = power;
            max_power_channel_idx = i;
        }

        if(tmp_max_norm > max_norm)
            max_norm = tmp_max_norm;

        // std::cout << max_norm << std::endl;

        // Graphics
        if constexpr(!HEADLESS) {
            NoteChannelData &ncd = tuned_graphics->get_note_channel_data();
            ncd.push_back(NoteChannelDataPoint(power));

            Spectrum &spectrum = tuned_graphics->get_spectrum();
            // if(i != 11)
            //     continue;

            // Start at j = 1 to skip rendering DC offset
            for(int j = 1; j < (buffer_sizes[i] / 2) + 1; j++)
                spectrum.add_data(j * ((double)SAMPLE_RATE / (double)buffer_sizes[i]), norms[j], (double)SAMPLE_RATE / (double)buffer_sizes[i]);
        }
    }
    if constexpr(!HEADLESS) {
        tuned_graphics->set_last_max_recorded_value(max_norm);

        Spectrum &spectrum = tuned_graphics->get_spectrum();
        spectrum.add_data(0.0, 0.0, 0.0);  // Make graph start at (0, 0)
        spectrum.sort();
    }

    perf.push_time_point("Norms calculated");


    note_events.push_back(NoteEvent(Note(LOWEST_NOTE.midi_number + max_power_channel_idx), buffer_sizes[0], 0));

    // if(max_norm > 10)
    //     note_events.push_back(NoteEvent(Note(Notes::E, 5), buffer_sizes[0], 0));
}
