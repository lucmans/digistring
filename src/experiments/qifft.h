#ifndef DIGISTRING_EXPERIMENTS_QIFFT_H
#define DIGISTRING_EXPERIMENTS_QIFFT_H


#include <fftw3.h>

#include <functional>


// Iteratively optimizes qxifft exponent to minimize frequency error
// Return best found exponent, or 0.0 on error
// Assumes HighRes Estimator like pitch estimation
double iteratively_optimize_qxifft(const int frame_size, const int padding_size/*, const window_func*/);


class QIFFT {
    public:
        QIFFT(const int _frame_size, const int _padding_size/*, const window_func*/);
        ~QIFFT();

        // Calculates different QIFFT errors empirically given an interpolation function
        // Returns mean squared error
        double qifft_error(const std::function<double(double, double, double, double&)> interpolation_func);
        // Shorthand so no function pointer has to be passed
        double no_qifft();  // Nearest bin method (no interpolation)
        double mqifft();
        double lqifft();
        double lqifft2();
        double lqifft10();
        double dbqifft();

        // Calculates XQIFFT errors empirically
        // Assumes HighRes Estimator like pitch estimation
        // Test for a single value; returns mean squared error or number below 0.0 on error
        double xqifft_exp_error(const double exp);
        // Returns best exponent, or 0.0 on error (0.0 is never a valid return value)
        // Optionally also gives error associated with best exponent and optionally prints all tested exponents and errors
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
