
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


class Estimator {
    public:
        Estimator();
        virtual ~Estimator();

        virtual Estimators get_type() const = 0;

        virtual float *get_input_buffer() const = 0;
        virtual float *get_input_buffer(int &buffer_size) const = 0;

        virtual void perform() = 0;


    protected:
        float *in;  // input buffer
};


#endif  // ESTIMATOR_H
