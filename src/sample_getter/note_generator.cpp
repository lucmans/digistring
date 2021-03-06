#include "note_generator.h"

#include "note.h"
#include "error.h"

#include "config/audio.h"
#include "config/transcription.h"

#include <cmath>
#include <iostream>


NoteGenerator::NoteGenerator(const int input_buffer_size, const Note &note) : SampleGetter(input_buffer_size), generated_note(note) {
    generated_note_number = generated_note.midi_number;

    last_phase = 0.0;
}

NoteGenerator::NoteGenerator(const int input_buffer_size, const int note_number) : SampleGetter(input_buffer_size), generated_note(note_number) {
    generated_note = Note(generated_note_number);

    last_phase = 0.0;
}

NoteGenerator::~NoteGenerator() {

}


SampleGetters NoteGenerator::get_type() const {
    return SampleGetters::note_generator;
}


void NoteGenerator::pitch_up() {
    generated_note_number++;
    generated_note = Note(generated_note_number);

    info("Playing note " + note_to_string(generated_note) + "  (" + STR(generated_note.freq) + " Hz)");
}

void NoteGenerator::pitch_down() {
    generated_note_number--;
    generated_note = Note(generated_note_number);

    info("Playing note " + note_to_string(generated_note) + "  (" + STR(generated_note.freq) + " Hz)");
}


int NoteGenerator::get_frame(float *const in, const int n_samples) {
    int overlap_n_samples = n_samples;
    float *overlap_in = in;
    if constexpr(DO_OVERLAP)
        calc_and_paste_overlap(overlap_in, overlap_n_samples);


    const double offset = (last_phase * ((double)SAMPLE_RATE / generated_note.freq));
    for(int i = 0; i < overlap_n_samples; i++)
        overlap_in[i] = sinf((2.0 * M_PI * ((double)i + offset) * generated_note.freq) / (double)SAMPLE_RATE);

    last_phase = fmod(last_phase + (generated_note.freq / ((double)SAMPLE_RATE / (double)overlap_n_samples)), 1.0);

    played_samples += overlap_n_samples;


    if constexpr(DO_OVERLAP)
        copy_overlap(in, n_samples);

    return overlap_n_samples;
}
