
#ifndef TUNED_H
#define TUNED_H


#include "estimator.h"


class Tuned : public Estimator {
    public:
        Tuned();
        ~Tuned();

        Estimators get_type() const override;

        float *get_input_buffer() const override;
        float *get_input_buffer(int &buffer_size) const override;

        void perform() override;


    private:
        // float *in;  // Moved to super class
};


#endif  // TUNED_H
