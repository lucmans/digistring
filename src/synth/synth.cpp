#include "synth.h"

#include "error.h"
#include "config/audio.h"

#include "SDL2/SDL.h"


Synth::Synth(const int _sample_rate /*= SAMPLE_RATE*/) {
    max_amp = 0.0;

    sample_rate = _sample_rate;

    if(AUDIO_FORMAT != AUDIO_F32SYS) {
        error("Synths currently do not support sample formats other than 32 bit floats");
        exit(EXIT_FAILURE);
    }
}

Synth::~Synth() {

}


void Synth::reset_max_amp() {
    max_amp = 0.0;
}


void Synth::set_max_amp(const double _max_amp) {
    max_amp = _max_amp;
}
