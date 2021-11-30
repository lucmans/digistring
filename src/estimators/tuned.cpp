
#include "tuned.h"

#include "../error.h"

#include "window_func.h"
#include "../note.h"

#include <fftw3.h>

#include <cmath>


inline int fourier_size(const Note &note) {
    return (int)round((double)SAMPLE_RATE / note.freq);
}


Tuned::Tuned(float *const input_buffer) {
    // error("Tuned class not yet implemented!");
    // exit(EXIT_FAILURE);

    // Input buffer is allocated by caller, as it is shared between multiple objects

    int o = 0;  // Octave offset
    for(int i = 0; i < 12; i++) {
        Notes new_note = static_cast<Notes>((LOWEST_NOTE.note + i) % 12);
        if(new_note == Notes::C)
            o++;

        buffer_sizes[i] = fourier_size(Note(new_note, LOWEST_NOTE.octave + o));
        // std::cout << buffer_sizes[i] << std::endl;
    }
    // std::cout << std::endl;

    ins[0] = input_buffer;
    for(int i = 1; i < 12; i++) {
        ins[i] = input_buffer + (buffer_sizes[0] - buffer_sizes[i]);

        // std::cout << ins[i] << std::endl;
    }
    // std::cout << std::endl;

    // Assign note letter to each buffer slot
    // for(int i = 0; i < 12; i++) {
    //     for(int j = 0; j < buffer_sizes[i]; j++) {
    //         ins[i][j] = i;
    //     }
    // }
    // for(int i = 0; i < buffer_sizes[0]; i++)
    //     std::cout << i << " " << ins[0][i] << std::endl;


    for(int i = 0; i < 12; i++) {
        outs[i] = (fftwf_complex*)fftwf_malloc((((buffer_sizes[i]) / 2) + 1) * sizeof(fftwf_complex));
        if(outs[i] == NULL) {
            error("Failed to malloc output buffer");
            exit(EXIT_FAILURE);
        }
    }

    // TODO: Exhaustive planner and graphics to show "optimizing planner"
    for(int i = 0; i < 12; i++) {
        // FFTW_PRESERVE_INPUT is important, as the input array is shared
        plans[i] = fftwf_plan_dft_r2c_1d(buffer_sizes[i], ins[i], outs[i], FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);

        // std::cout << "Plan " << i << std::endl;
    }

    // Pre-calculate window functions
    for(int i = 0; i < 12; i++) {
        window_funcs[i] = (double*)malloc(buffer_sizes[i] * sizeof(double));
        if(window_funcs[i] == NULL) {
            error("Failed to allocate buffer for window function");
            exit(EXIT_FAILURE);
        }

        blackman_nuttall_window(window_funcs[i], buffer_sizes[i]);
    }
}

Tuned::~Tuned() {
    for(int i = 0; i < 12; i++)
        fftwf_destroy_plan(plans[i]);

    for(int i = 0; i < 12; i++)
        fftwf_free(outs[i]);

    for(int i = 0; i < 12; i++)
        free(window_funcs[i]);
}


Estimators Tuned::get_type() const {
    return Estimators::tuned;
}


/*static*/ float *Tuned::create_input_buffer(int &buffer_size) {
    const int n_samples = fourier_size(LOWEST_NOTE);

    float *input_buffer = (float*)fftwf_malloc(n_samples * sizeof(float));
    if(input_buffer == NULL) {
        error("Failed to malloc input buffer");
        exit(EXIT_FAILURE);
    }

    buffer_size = n_samples;
    return input_buffer;
}

float *Tuned::_create_input_buffer(int &buffer_size) const {
    return Tuned::create_input_buffer(buffer_size);
}

// Implemented by superclass
// void HighRes::free_input_buffer(float *const input_buffer) const {
//     fftwf_free(input_buffer);
// }


void Tuned::perform(float *const input_buffer) {
    // TODO
    warning("Tuned 'perform' not yet implemented");

    input_buffer[0] = 0.0;  // prevent warning
}
