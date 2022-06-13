#ifndef DIGISTRING_EXPERIMENTS_QIFFT_H
#define DIGISTRING_EXPERIMENTS_QIFFT_H


#include <fftw3.h>


// Assumes HighRes Estimator like pitch estimation
double iteratively_optimize_qxifft(const int frame_size, const int padding_size/*, const window_func*/);


class QIFFT {
    public:
        QIFFT(const int _frame_size, const int _padding_size/*, const window_func*/);
        ~QIFFT();

        // Error of nearest bin method (no interpolation)
        double no_qifft();

        // Calculates different QIFFT errors empirically
        // Return average error in Hz
        double mqifft();
        double lqifft();
        double lqifft2();
        double lqifft10();
        double dbqifft();

        // Calculates XQIFFT errors empirically
        // Assumes HighRes Estimator like pitch estimation
        // Test for a single value; returns Hz error or number below 0.0 on error
        double xqifft_exp_error(const double exp);
        // Returns best exponent, or 0.0 on error (0.0 is never a valid return value)
        double xqifft_exp_test_range(const double check_min, const double check_max, const double check_step, const bool print = false);
        double xqifft_exp_test_range(const double check_min, const double check_max, const double check_step, double &out_error, const bool print = false);


    private:
        const int frame_size;
        const int padding_size;
        const int in_size;

        float *in;
        fftwf_complex *out;
        fftwf_plan p;

        float *window_func;
        double *norms;
};


#endif  // DIGISTRING_EXPERIMENTS_QIFFT_H
