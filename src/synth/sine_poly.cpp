#include "sine_poly.h"

#include "note.h"
#include "error.h"
#include "config/synth.h"

#include <cmath>
#include <algorithm>  // std::fill_n()


SinePoly::SinePoly() {
    warning("The polyphonic sine synth is not yet finished and produces plops");
}

SinePoly::~SinePoly() {

}


void SinePoly::zero_note(const double freq, const double amp, const double phase, const int offset, float *const synth_buffer, const int n_samples) {
    const double phase_offset = (phase * ((double)sample_rate / freq));

    int i;
    for(i = 0; i < n_samples - offset; i++) {
        const double next_sample = amp * sinf((2.0 * M_PI * ((double)i + phase_offset) * freq) / (double)sample_rate);
        if(phase > 0.5 && next_sample >= 0.0)
            break;
        else if(phase <= 0.5 && next_sample <= 0.0)
            break;

        synth_buffer[i + offset] += next_sample;
    }

    if(i == n_samples) {
        // const double last_phase = fmod(phase + (freq / ((double)sample_rate / (double)i)), 1.0);

        warning("Failed to zero note within frame");
        // TODO: Zero next frame
    }
}


void SinePoly::place_sine(const NoteEvent note_event, float *const synth_buffer, const int n_samples, const double amp, const double start_phase /*= 0.0*/) {
    // DEBUG: Sanity check
    if(note_event.offset + note_event.length > n_samples) {
        error("Note event passed to synthesizer is longer than the synth_buffer, or the synth_buffer was created shorter than input_buffer_n_samples or Estimator gave note event information from beyond its buffer.");
        exit(EXIT_FAILURE);
    }

    const double phase_offset = start_phase * ((double)sample_rate / note_event.note.freq);

    int i;
    for(i = 0; i < note_event.length; i++)
        synth_buffer[i + note_event.offset] += amp * sinf((2.0 * M_PI * ((double)i + phase_offset) * note_event.note.freq) / (double)sample_rate);

    const double last_phase = fmod(start_phase + (note_event.note.freq / ((double)sample_rate / (double)note_event.length)), 1.0);
    // Deal with rest of this note next frame
    if(i == n_samples) {
        next_prev_frame_notes.push_back(PrevFrameNote(note_event.note, last_phase));
        return;
    }

    // Note ended within frame, so zero it
    zero_note(note_event.note.freq, amp, last_phase, note_event.offset + note_event.length, synth_buffer, n_samples);
}


void SinePoly::synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples, const double volume /*= 1.0*/) {
    std::fill_n(synth_buffer, n_samples, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles

    NoteEvents todo = note_events;
    const int n_todo = note_events.size();

    // Connect sine of same note with previous frame
    const int n_prev_frame_notes = prev_frame_notes.size();
    for(int i = 0; i < n_prev_frame_notes; i++) {
        for(int j = 0; j < n_todo; j++) {
            // If note doesn't start at the start of buffer, it is separate from the previous frame
            if(todo[j].offset != 0)
                continue;

            const double cent_difference = 1200.0 * log2(prev_frame_notes[i].note.freq / todo[j].note.freq);
            if(std::abs(cent_difference) < MAX_CENT_DIFFERENCE) {
                place_sine(todo[j], synth_buffer, n_samples, volume, prev_frame_notes[i].end_phase);
                todo[j].length = -1;
                todo[j].offset = -1;
                prev_frame_notes[i].end_phase = -1.0;
                break;
            }
        }

        // No note in current frame matching note in prev frame, so zero
        if(prev_frame_notes[i].end_phase >= 0)
            zero_note(prev_frame_notes[i].note.freq, volume, prev_frame_notes[i].end_phase, 0, synth_buffer, n_samples);
    }

    // Generate all new note sines
    for(int i = 0; i < n_todo; i++) {
        // Already done above as it matched a note in previous frame
        if(todo[i].offset == -1)
            continue;

        place_sine(todo[i], synth_buffer, n_samples, volume);
    }

    // Normalize volume when clipping
    int max_idx = 0;
    for(int i = 0; i < n_samples; i++)
        if(std::abs(synth_buffer[i]) > std::abs(synth_buffer[max_idx]))
            max_idx = i;

    const double max_val = std::abs(synth_buffer[max_idx]);
    if(max_val > 1.0)
        for(int i = 0; i < n_samples; i++)
            synth_buffer[i] /= max_val;

    //
    prev_frame_notes = std::move(next_prev_frame_notes);
}
