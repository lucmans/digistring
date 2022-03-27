#ifndef DIGISTRING_SYNTH_SQUARE_H
#define DIGISTRING_SYNTH_SQUARE_H


#include "synth.h"

#include "note.h"


class Square : public Synth {
    public:
        Square();
        ~Square() override;

        void synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples);


    private:
        double last_phase;
};


#endif  // DIGISTRING_SYNTH_SQUARE_H
