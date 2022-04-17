#include "synth.h"


Synth::Synth() {
    max_amp = 0.0;
}

Synth::~Synth() {

}


void Synth::reset_max_amp() {
    max_amp = 0.0;
}


void Synth::set_max_amp(const double _max_amp) {
    max_amp = _max_amp;
}
