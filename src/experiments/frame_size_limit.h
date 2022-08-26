#ifndef DIGISTRING_EXPERIMENTS_FRAME_SIZE_LIMIT_H
#define DIGISTRING_EXPERIMENTS_FRAME_SIZE_LIMIT_H


#include <fftw3.h>


class FrameSizeLimit {
    public:
        FrameSizeLimit(const int _frame_size, const int _padding_size/*, const window_func*/);
        ~FrameSizeLimit();

        void test();


    private:
        const int frame_size;
        const int padding_size;
        const int in_size;

        float *in;
        fftwf_complex *out;
        fftwf_plan p;

        float *window_func;
        double *norms;
};


#endif  // DIGISTRING_EXPERIMENTS_FRAME_SIZE_LIMIT_H
