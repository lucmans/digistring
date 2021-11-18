
#ifndef HIGHRES_H
#define HIGHRES_H


#include "estimator.h"
#include "../config.h"

#include <fftw3.h>


class HighRes : public Estimator {
    public:
        HighRes();
        ~HighRes();

        Estimators get_type() const override;

        float *get_input_buffer() const override;
        float *get_input_buffer(int &buffer_size) const override;

        void perform() override;


    private:
        // float *in;  // Moved to super class
        fftwf_complex *out;
        fftwf_plan p;

        double window_func[FRAME_SIZE];
};


#endif  // HIGHRES_H
