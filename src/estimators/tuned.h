
#ifndef TUNED_H
#define TUNED_H


#include "estimator.h"

#include <fftw3.h>

#include "../note.h"


class Tuned : public Estimator {
    public:
        Tuned(float *const input_buffer);
        ~Tuned() override;

        Estimators get_type() const override;

        // The input buffer has to be freed by caller using fftwf_free()
        static float *create_input_buffer(int &buffer_size);
        float *_create_input_buffer(int &buffer_size) const override;

        // Implemented by superclass
        // void free_input_buffer(float *const input_buffer) const {

        void perform(float *const input_buffer, NoteSet &noteset) override;


    private:
        int buffer_sizes[12];
        float *ins[12];
        fftwf_complex *outs[12];
        fftwf_plan plans[12];

        double *window_funcs[12];
};


#endif  // TUNED_H
