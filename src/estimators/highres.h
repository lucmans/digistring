
#ifndef HIGHRES_H
#define HIGHRES_H


#include "estimator.h"

#include "../config.h"

#include <fftw3.h>


class HighRes : public Estimator {
    public:
        HighRes(float *const input_buffer);
        ~HighRes();

        Estimators get_type() const override;

        // The input buffer has to be freed by caller using fftwf_free()
        static float *create_input_buffer(int &buffer_size);
        float *_create_input_buffer(int &buffer_size) const override;

        // Implemented by superclass
        // void free_input_buffer(float *const input_buffer) const;

        void perform(float *const input_buffer, NoteSet &noteset) override;


    private:
        fftwf_complex *out;
        fftwf_plan p;

        double window_func[FRAME_SIZE];
};


#endif  // HIGHRES_H
