#ifndef DIGISTRING_ESTIMATORS_TUNED_H
#define DIGISTRING_ESTIMATORS_TUNED_H


#include "estimator.h"

#include <fftw3.h>

#include "note.h"


class Tuned : public Estimator {
    public:
        Tuned(float *&input_buffer, int &buffer_size);
        ~Tuned() override;

        Estimators get_type() const override;

        void perform(float *const input_buffer, NoteEvents &note_events) override;


    private:
        int buffer_sizes[12];
        float *ins[12];
        fftwf_complex *outs[12];
        fftwf_plan plans[12];

        double *norms;
        float *window_funcs[12];
};


#endif  // DIGISTRING_ESTIMATORS_TUNED_H
