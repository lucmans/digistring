#ifndef DIGISTRING_SYNTH_SYNTHS_H
#define DIGISTRING_SYNTH_SYNTHS_H


// Base class
#include "synth.h"

// Basic waveform synths
#include "square.h"
#include "sine.h"
#include "sine_amped.h"


Synth *synth_factory(const Synths &synth_type);


#endif  // DIGISTRING_SYNTH_SYNTHS_H
