#include "sine.h"

#include "note.h"
#include "error.h"

#include <cmath>
#include <algorithm>  // std::fill_n()


Sine::Sine() {
    last_phase = 0.0;

    prev_frame_silent = true;
}

Sine::~Sine() {

}


void Sine::synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples, const double volume /*= 1.0*/) {
    const int n_events = note_events.size();

    // Output silence if there are no notes
    if(n_events == 0) {
        std::fill_n(synth_buffer, n_samples, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles

        // Finish sine from previous frame
        if(!prev_frame_silent) {
            const double phase_offset = (last_phase * ((double)sample_rate / prev_frame_freq));
            if(last_phase > 0.5) {
                for(int i = 0; i < n_samples; i++) {
                    const double next_sample = sinf((2.0 * M_PI * ((double)i + phase_offset) * prev_frame_freq) / (double)sample_rate);
                    if(next_sample > 0.0)
                        break;

                    synth_buffer[i] = volume * next_sample;
                }
            }
            else /*if(last_phase < 0.5)*/ {
                for(int i = 0; i < n_samples; i++) {
                    const double next_sample = sinf((2.0 * M_PI * ((double)i + phase_offset) * prev_frame_freq) / (double)sample_rate);
                    if(next_sample < 0.0)
                        break;

                    synth_buffer[i] = volume * next_sample;
                }
            }
        }

        prev_frame_silent = true;
        last_phase = 0.0;
        return;
    }

    // Find relevant event from note_events
    int event_idx = 0;
    if(n_events > 1) {
        warning("Polyphony not yet supported; playing loudest note instead");  // TODO: Support
        for(int i = 1; i < n_events; i++)
            if(note_events[i].note.amp > note_events[event_idx].note.amp)
                event_idx = i;
    }
    const NoteEvent &out_event = note_events[event_idx];

    // DEBUG: Sanity check
    if(out_event.offset + out_event.length > n_samples) {
        error("Note event passed to synthesizer is longer than the synth_buffer\nOr the synth_buffer was created shorter than input_buffer_n_samples or Estimator gave note event information from beyond its buffer.");
        exit(EXIT_FAILURE);
    }

    // Always make signal start with 0 if silence before
    if(prev_frame_silent)
        last_phase = 0.0;
    prev_frame_silent = false;

    // Write samples to buffer
    const Note &out_note = out_event.note;
    const double phase_offset = last_phase * ((double)sample_rate / out_note.freq);
    for(int i = out_event.offset; i < out_event.offset + out_event.length; i++)
        synth_buffer[i] = volume * sinf((2.0 * M_PI * ((double)i + phase_offset) * out_note.freq) / (double)sample_rate);

    last_phase = fmod(last_phase + (out_note.freq / ((double)sample_rate / (double)n_samples)), 1.0);

    // Fill silent part with zeros
    std::fill_n(synth_buffer, out_event.offset, 0.0);  // Start
    std::fill_n(synth_buffer + out_event.offset + out_event.length, n_samples - (out_event.offset + out_event.length), 0.0);  // End

    prev_frame_freq = out_note.freq;
}
