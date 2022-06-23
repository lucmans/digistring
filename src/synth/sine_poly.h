#ifndef DIGISTRING_SYNTH_SINE_POLY_H
#define DIGISTRING_SYNTH_SINE_POLY_H


#include "synth.h"

#include "note.h"

#include <vector>


struct PrevFrameNote {
    Note note;
    double end_phase;

    PrevFrameNote(const Note &_note, const double _end_phase) : note(_note), end_phase(_end_phase) {};
};

typedef std::vector<PrevFrameNote> PrevFrameNotes;


class SinePoly : public Synth {
    public:
        SinePoly();
        ~SinePoly() override;

        void zero_note(const double freq, const double amp, const double phase, const int offset, float *const synth_buffer, const int n_samples);
        void place_sine(const NoteEvent note, float *const synth_buffer, const int n_samples, const double amp, const double start_phase = 0.0);

        void synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples, const double volume = 1.0);


    private:
        PrevFrameNotes prev_frame_notes;
        PrevFrameNotes next_prev_frame_notes;
        // std::vector<double> notes_to_zero;
};


#endif  // DIGISTRING_SYNTH_SINE_POLY_H
