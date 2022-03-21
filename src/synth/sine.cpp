#include "sine.h"

#include "note.h"
#include "error.h"
#include "config/audio.h"


Sine::Sine() {
    last_phase = 0.0;
}

Sine::~Sine() {

}


// TODO: Support NoteEvent timing data instead of playing the note the whole buffer
void Sine::synthesize(const NoteEvents &notes, float *const synth_buffer, const int n_samples) {
    // Output silence if there are no notes
    if(notes.size() == 0) {
        for(int i = 0; i < n_samples; i++)
            synth_buffer[i] = 0.0;

        return;
    }

    if(notes.size() > 1) {
        warning("Polyphony not yet supported");  // TODO: Support
        for(int i = 0; i < n_samples; i++)
            synth_buffer[i] = 0.0;

        return;
    }

    const double offset = (last_phase * ((double)SAMPLE_RATE / notes[0].note.freq));
    for(int i = 0; i < n_samples; i++)
        synth_buffer[i] = sinf((2.0 * M_PI * ((double)i + offset) * notes[0].note.freq) / (double)SAMPLE_RATE);

    last_phase = fmod(last_phase + (notes[0].note.freq / ((double)SAMPLE_RATE / (double)n_samples)), 1.0);
}
