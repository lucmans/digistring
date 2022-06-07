#include "norms.h"

#include <fftw3.h>

#include <cmath>


// Normalize results: http://fftw.org/fftw3_doc/The-1d-Discrete-Fourier-Transform-_0028DFT_0029.html
void calc_norms(const fftwf_complex values[], double norms[], const int n_norms) {
    for(int i = 0; i < n_norms; i++)
        norms[i] = sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1]));
}

void calc_norms(const fftwf_complex values[], double norms[], const int n_norms, double &max_norm, double &power) {
    max_norm = -1.0;
    power = 0.0;

    for(int i = 0; i < n_norms; i++) {
        norms[i] = sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1]));
        power += norms[i];

        if(norms[i] > max_norm)
            max_norm = norms[i];
    }
}


// dB ref: https://www.kvraudio.com/forum/viewtopic.php?t=276092
void calc_norms_db(const fftwf_complex values[], double norms[], const int n_norms) {
    for(int i = 0; i < n_norms; i++)
        // norms[i] = 20.0 * log10(sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
        norms[i] = 20.0 * log10(1.0 + sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
        // norms[i] = 20.0 * log10(2.0 * sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])) / n);
}

void calc_norms_db(const fftwf_complex values[], double norms[], const int n_norms, double &max_norm, double &power) {
    max_norm = -1.0;
    power = 0.0;

    for(int i = 0; i < n_norms; i++) {
        // norms[i] = 20.0 * log10(sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
        norms[i] = 20.0 * log10(1.0 + sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
        // norms[i] = 20.0 * log10(2.0 * sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])) / n);
        power += norms[i];

        if(norms[i] > max_norm)
            max_norm = norms[i];
    }
}
