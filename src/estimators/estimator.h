
#ifndef ESTIMATOR_H
#define ESTIMATOR_H


#include "../note.h"
#include "../spectrum.h"

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


class Estimator {
    public:
        Estimator();
        virtual ~Estimator();

        // This function should only return its type as named in Estimators
        virtual Estimators get_type() const = 0;

        // Creates (SIMD) aligned input buffer of correct size
        // The input buffer has to be freed by caller using free_input_buffer()
        // The following function has to be implemented by every inherited class
        /* static float *HighRes::create_input_buffer(int &buffer_size); */
        // The next function "forces" everyone to implement the static method
        virtual float *_create_input_buffer(int &buffer_size) const = 0;
        // And the function to free it
        static void free_input_buffer(float *const input_buffer);

        // Functions for retrieving graphics related data
        double get_max_norm() const;
        // The spectrum data pointer is valid till the next perform() call
        // It also sorts the spectrum, as is needed for the graphics
        const Spectrum *get_spectrum();

        // Actually performs the estimation
        virtual void perform(float *const input_buffer, NoteSet &noteset) = 0;


    protected:
        // Graphics output related variables, so only available in without headless mode
        // These variables are set during perform, so relate to the last perform call
        Spectrum spectrum;
        double max_norm;
};


#endif  // ESTIMATOR_H
