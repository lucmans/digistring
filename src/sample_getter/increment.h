
#ifndef INCREMENT_H
#define INCREMENT_H


#include "sample_getter.h"


// For debugging purposes
// Given the i-th generated sample s_i, the value of each sample s_i = s_(i-1) + 1 with s_0 = 1
// In other words, first sample is 1, every next sample is +1
class Increment : public SampleGetter {
    public:
        Increment();
        ~Increment() override;

        SampleGetters get_type() const override;

        void get_frame(float *const in, const int n_samples);


    private:
        //
};


#endif  // INCREMENT_H
