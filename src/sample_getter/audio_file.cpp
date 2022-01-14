
#include "audio_file.h"

#include "../error.h"


AudioFile::AudioFile() {

}

AudioFile::~AudioFile() {

}


SampleGetters AudioFile::get_type() const {
    return SampleGetters::audio_file;
}


void AudioFile::get_frame(float *const in, const int n_samples) {
    static bool shown = false;
    if(!shown) {
        warning("Not yet implemented!");
        shown = !shown;
    }

    // TODO
    for(int i = 0; i < n_samples; i++)
        in[i] = 0;
}
