#include "config.h"

#include <fftw3.h>

#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <chrono>
#include <cmath>


int main(int argc, char *argv[]) {
    std::chrono::steady_clock::time_point start, stop;
    std::chrono::duration<double> dur;

    double *in = (double*)fftw_malloc(TRANSFORM_SIZE * sizeof(double));
    if(in == NULL) {
        std::cout << "Error: Failed to allocate input buffer" << std::endl;
        exit(EXIT_FAILURE);
    }

    fftw_complex *out = (fftw_complex*)fftw_malloc(((TRANSFORM_SIZE / 2) + 1) * sizeof(fftw_complex));
    if(out == NULL) {
        std::cout << "Error: Failed to allocate output buffer" << std::endl;
        exit(EXIT_FAILURE);
    }

    fftw_plan p = fftw_plan_dft_r2c_1d(TRANSFORM_SIZE, in, out, FFTW_ESTIMATE);
    if(p == NULL) {
        std::cout << "Error: Failed to create FFTW3 plan" << std::endl;
        exit(EXIT_FAILURE);
    }


    // Fill in with data
    for(int i = 0; i < TRANSFORM_SIZE; i++)
        in[i] = sin((2.0 * (double)M_PI * (double)i * FREQ) / (double)TRANSFORM_SIZE);

    // Transform
    start = std::chrono::steady_clock::now();
    for(int i = 0; i < CYCLES; i++)
        fftw_execute(p);    
    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "Performed " << CYCLES << " Fourier transforms with " << TRANSFORM_SIZE << " samples\n"
              << "Total time: " << dur.count() << " seconds\n"
              << std::endl;


    fftw_destroy_plan(p);
    fftw_free(out);
    fftw_free(in);

    return EXIT_SUCCESS;
}
