#ifndef DIGISTRING_SAMPLE_GETTER_NOTE_GENERATOR_H
#define DIGISTRING_SAMPLE_GETTER_NOTE_GENERATOR_H


#include "sample_getter.h"

#include "note.h"


class NoteGenerator : public SampleGetter {
    public:
        NoteGenerator(const Note &note);
        NoteGenerator(const int note_number);
        ~NoteGenerator() override;

        SampleGetters get_type() const override;

        void pitch_up() override;
        void pitch_down() override;

        void get_frame(float *const in, const int n_samples);


    private:
        Note generated_note;
        int generated_note_number;

        double last_phase;
};


#endif  // DIGISTRING_SAMPLE_GETTER_NOTE_GENERATOR_H
