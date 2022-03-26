#ifndef DIGISTRING_SYNTH_SINE_H
#define DIGISTRING_SYNTH_SINE_H


#include "synth.h"

#include "note.h"


class Sine : public Synth {
    public:
        Sine();
        ~Sine() override;

        void synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples);


    private:
        double last_phase;
};


#endif  // DIGISTRING_SYNTH_SINE_H
