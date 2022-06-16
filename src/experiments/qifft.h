#ifndef DIGISTRING_EXPERIMENTS_QIFFT_H
#define DIGISTRING_EXPERIMENTS_QIFFT_H


#include <fftw3.h>

#include <functional>
#include <string>


// Iteratively optimizes qxifft exponent to minimize frequency error
// Return best found exponent, or 0.0 on error
// Assumes HighRes Estimator like pitch estimation
double iteratively_optimize_qxifft(const int frame_size, const int padding_size/*, const window_func*/);


// Error measures should be >=0.0
struct ErrorMeasures {
    double mean_error;
    double mean_squared_error;
    double max_error;

    ErrorMeasures() : mean_error(-1.0), mean_squared_error(-1.0), max_error(-1.0) {};  // Default error/invalid values
    ErrorMeasures(const double _mean_error, const double _mean_squared_error, const double _max_error)
        : mean_error(_mean_error), mean_squared_error(_mean_squared_error), max_error(_max_error) {};
};

// Using this function, we can easily change the error measure which is minimized/returned from the QIFFT functions
const std::string OPTI_MEASURE_STR = "mean squared error";
const std::string OPTI_MEASURE_UNIT_STR = "";  // Should provide leading space
inline double get_opti_measure(ErrorMeasures errors) {
    return errors.mean_squared_error;
}


class QIFFT {
    public:
        QIFFT(const int _frame_size, const int _padding_size/*, const window_func*/);
        ~QIFFT();

        // Calculates different QIFFT errors empirically given an interpolation function
        // Returns mean squared error
        ErrorMeasures qifft_error(const std::function<double(double, double, double, double&)> interpolation_func);
        // Shorthand so no function pointer has to be passed
        ErrorMeasures no_qifft();  // Nearest bin method (no interpolation)
        ErrorMeasures mqifft();
        ErrorMeasures lqifft();
        ErrorMeasures lqifft2();
        ErrorMeasures lqifft10();
        ErrorMeasures dbqifft();

        // Calculates XQIFFT errors empirically
        // Assumes HighRes Estimator like pitch estimation
        // Test for a single value; returns errors below 0.0 on error
        ErrorMeasures xqifft_exp_error(const double exp);
        // Returns best exponent, or 0.0 on error (0.0 is never a valid return value)
        // Optionally also gives error associated with best exponent and optionally prints all tested exponents and errors
        double xqifft_exp_test_range(const double check_min, const double check_max, const double check_step, const bool print = false);
        double xqifft_exp_test_range(const double check_min, const double check_max, const double check_step, ErrorMeasures &out_error, const bool print = false);


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
