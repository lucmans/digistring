
#include "estimation_func.h"

#include <fftw3.h>

#include <cmath>


// Normalize results: http://fftw.org/fftw3_doc/The-1d-Discrete-Fourier-Transform-_0028DFT_0029.html
void calc_norms(const fftwf_complex values[], double norms[], const int n) {
    for(int i = 1; i < n; i++)
        norms[i] = sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1]));
}

void calc_norms(const fftwf_complex values[], double norms[], const int n, double &max_norm, double &power) {
    max_norm = -1.0;
    power = 0.0;

    for(int i = 1; i < n; i++) {
        norms[i] = sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1]));
        power += norms[i];

        if(norms[i] > max_norm)
            max_norm = norms[i];
    }
}

// dB ref: https://www.kvraudio.com/forum/viewtopic.php?t=276092
void calc_norms_db(const fftwf_complex values[], double norms[], const int n) {
    for(int i = 1; i < n; i++)
        norms[i] = 20 * log10(sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
}

void calc_norms_db(const fftwf_complex values[], double norms[], const int n, double &max_norm, double &power) {
    max_norm = -1.0;
    power = 0.0;

    for(int i = 1; i < n; i++) {
        norms[i] = 20 * log10(sqrt((values[i][0] * values[i][0]) + (values[i][1] * values[i][1])));
        power += norms[i];

        if(norms[i] > max_norm)
            max_norm = norms[i];
    }
}
