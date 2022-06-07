#ifndef DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_NORMS_H
#define DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_NORMS_H


#include <fftw3.h>


// Normalize results: http://fftw.org/fftw3_doc/The-1d-Discrete-Fourier-Transform-_0028DFT_0029.html
void calc_norms(const fftwf_complex values[], double norms[], const int n_norms);
void calc_norms(const fftwf_complex values[], double norms[], const int n_norms, double &max_norm, double &power);

// dB ref: https://www.kvraudio.com/forum/viewtopic.php?t=276092
void calc_norms_db(const fftwf_complex values[], double norms[], const int n_norms);
void calc_norms_db(const fftwf_complex values[], double norms[], const int n_norms, double &max_norm, double &power);


#endif  // DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_NORMS_H
