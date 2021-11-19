
#ifndef TUNED_H
#define TUNED_H


#include "estimator.h"


class Tuned : public Estimator {
    public:
        Tuned(float *const input_buffer);
        ~Tuned();

        Estimators get_type() const override;

        // float *get_input_buffer() const override;
        // float *get_input_buffer(int &buffer_size) const override;

        // The input buffer has to be freed by caller using fftwf_free()
        static float *create_input_buffer(int &buffer_size);
        float *_create_input_buffer(int &buffer_size) const override;

        // Implemented by superclass
        // void free_input_buffer(float *const input_buffer) const {

        void perform(float *const input_buffer) override;


    private:
        // float *in;  // Moved to super class
};


#endif  // TUNED_H
