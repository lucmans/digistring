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
void Sine::synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples) {
    const int n_events = note_events.size();

    // Output silence if there are no notes
    if(n_events == 0) {
        for(int i = 0; i < n_samples; i++)
            synth_buffer[i] = 0.0;

        return;
    }

    int note_idx = 0;
    if(n_events > 1) {
        warning("Polyphony not yet supported; playing loudest note instead");  // TODO: Support
        for(int i = 1; i < n_events; i++)
            if(note_events[i].note.amp > note_events[note_idx].note.amp)
                note_idx = i;
    }

    const double offset = (last_phase * ((double)SAMPLE_RATE / note_events[note_idx].note.freq));
    for(int i = 0; i < n_samples; i++)
        synth_buffer[i] = sinf((2.0 * M_PI * ((double)i + offset) * note_events[note_idx].note.freq) / (double)SAMPLE_RATE);

    last_phase = fmod(last_phase + (note_events[note_idx].note.freq / ((double)SAMPLE_RATE / (double)n_samples)), 1.0);
}
