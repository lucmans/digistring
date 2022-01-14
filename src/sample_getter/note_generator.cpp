
#include "note_generator.h"

#include "../note.h"
#include "../config.h"
#include "../error.h"

#include <cmath>
#include <iostream>


NoteGenerator::NoteGenerator(const Note &note) : generated_note(note) {
    last_phase = 0.0;
}

NoteGenerator::~NoteGenerator() {

}


SampleGetters NoteGenerator::get_type() const {
    return SampleGetters::note_generator;
}


void NoteGenerator::pitch_up() {
    if(generated_note.note == Notes::B)
        generated_note = Note(Notes::C, generated_note.octave + 1);
    else
        generated_note = Note(static_cast<Notes>(static_cast<int>(generated_note.note) + 1), generated_note.octave);

    std::cout << "Playing note " << generated_note << "  (" << generated_note.freq << " Hz)" << std::endl;
}

void NoteGenerator::pitch_down() {
    if(generated_note.note == Notes::C)
        generated_note = Note(Notes::B, generated_note.octave - 1);
    else
        generated_note = Note(static_cast<Notes>(static_cast<int>(generated_note.note) - 1), generated_note.octave);

    std::cout << "Playing note " << generated_note << "  (" << generated_note.freq << " Hz)" << std::endl;
}


void NoteGenerator::get_frame(float *const in, const int n_samples) {
    for(int i = 0; i < n_samples; i++) {
        const double offset = (last_phase * ((double)SAMPLE_RATE / generated_note.freq));
        in[i] = sinf((2.0 * M_PI * ((double)i + offset) * generated_note.freq) / (double)SAMPLE_RATE);
    }
    last_phase = fmod(last_phase + (generated_note.freq / ((double)SAMPLE_RATE / (double)n_samples)), 1.0);
}
