#include "config.h"

#include <fftw3.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <chrono>
#include <cmath>


int main(int argc, char *argv[]) {
    std::chrono::steady_clock::time_point start, stop;
    std::chrono::duration<double> dur;

    float *in = (float*)fftwf_malloc(TRANSFORM_SIZE * sizeof(float));
    if(in == NULL) {
        std::cout << "Error: Failed to allocate input buffer" << std::endl;
        exit(EXIT_FAILURE);
    }

    fftwf_complex *out = (fftwf_complex*)fftwf_malloc(((TRANSFORM_SIZE / 2) + 1) * sizeof(fftwf_complex));
    if(out == NULL) {
        std::cout << "Error: Failed to allocate output buffer" << std::endl;
        exit(EXIT_FAILURE);
    }

    fftwf_plan p = fftwf_plan_dft_r2c_1d(TRANSFORM_SIZE, in, out, FFTW_ESTIMATE);
    if(p == NULL) {
        std::cout << "Error: Failed to create FFTW3 plan" << std::endl;
        exit(EXIT_FAILURE);
    }


    // Fill in with data
    for(int i = 0; i < TRANSFORM_SIZE; i++)
        in[i] = sinf((2.0f * (float)M_PI * (float)i * (float)FREQ) / (float)TRANSFORM_SIZE);

    // Transform
    start = std::chrono::steady_clock::now();
    for(int i = 0; i < CYCLES; i++)
        fftwf_execute(p);
    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "Performed " << CYCLES << " Fourier transforms with " << TRANSFORM_SIZE << " samples\n"
              << "Total time: " << dur.count() << " seconds\n"
              << std::endl;


    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in);

    return EXIT_SUCCESS;
}
