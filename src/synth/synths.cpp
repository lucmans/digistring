#include "synths.h"

#include "square.h"
#include "sine.h"
#include "sine_amped.h"
#include "sine_poly.h"

#include "error.h"


Synth *synth_factory(const Synths &synth_type) {
    switch(synth_type) {
        case Synths::square:
            return new Square();

        case Synths::sine:
            return new Sine();

        case Synths::sine_amped:
            return new SineAmped();

        case Synths::sine_poly:
            return new SinePoly();

        default:
            error("Synth factory doesn't recognize synth type");
            exit(EXIT_FAILURE);
    }
}
