#ifndef DIGISTRING_SAMPLE_GETTER_INCREMENT_H
#define DIGISTRING_SAMPLE_GETTER_INCREMENT_H


#include "sample_getter.h"


// For debugging purposes
// Given the i-th generated sample s_i, the value of each sample s_i = s_(i-1) + 1 with s_0 = 1
// In other words, first sample is 1, every next sample is +1
class Increment : public SampleGetter {
    public:
        Increment(const int input_buffer_size);
        ~Increment() override;

        SampleGetters get_type() const override;

        bool is_audio_recording_device() const override;

        void calc_and_paste_nonblocking_overlap(float *&in, int &n_samples);
        void copy_nonblocking_overlap(float *const in, const int n_samples);

        int get_frame(float *const in, const int n_samples);


    private:
        //
};


#endif  // DIGISTRING_SAMPLE_GETTER_INCREMENT_H
