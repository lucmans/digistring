
#ifndef AUDIO_FILE
#define AUDIO_FILE


#include "sample_getter.h"


class AudioFile : public SampleGetter {
    public:
        AudioFile();
        ~AudioFile() override;

        SampleGetters get_type() const override;

        void get_frame(float *const in, const int n_samples);


    private:
        //
};


#endif  // AUDIO_FILE
