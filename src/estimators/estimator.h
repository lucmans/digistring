
#ifndef ESTIMATOR_H
#define ESTIMATOR_H


#include <map>
#include <string>


/* When adding a new estimator, don't forget to include the file in estimators.h */

// Different estimator algorithms types
enum class Estimators {
    highres, tuned
};

// For printing enum
// If a string isn't present in EstimatorString for every type, random crashes may happen
extern const std::map<const Estimators, const std::string> EstimatorString;


#include "../config.h"
class Estimator {
    public:
        Estimator();
        virtual ~Estimator();

        virtual Estimators get_type() const = 0;

        // Creates (SIMD) aligned input buffer of correct size
        // The input buffer has to be freed by caller using free_input_buffer()
        // The following function has to be implemented by every inherited class
        /* static float *HighRes::create_input_buffer(int &buffer_size); */
        // The next function "forces" everyone to implement the static method
        virtual float *_create_input_buffer(int &buffer_size) const = 0;

        void free_input_buffer(float *const input_buffer) const;

        double get_max_norm() const;
        void get_data_point(const double *&out_norms, int &norms_size) const;

        virtual void perform(float *const input_buffer) = 0;


    protected:
        // Graphics output related variables
        double norms[(FRAME_SIZE / 2) + 1];
        double max_norm;
};


#endif  // ESTIMATOR_H
