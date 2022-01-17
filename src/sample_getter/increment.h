
#ifndef INCREMENT_H
#define INCREMENT_H


#include "sample_getter.h"


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
