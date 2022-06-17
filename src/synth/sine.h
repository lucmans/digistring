#ifndef DIGISTRING_SYNTH_SINE_H
#define DIGISTRING_SYNTH_SINE_H


#include "synth.h"

#include "note.h"


class Sine : public Synth {
    public:
        Sine();
        ~Sine() override;

        void synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples, const double volume = 1.0);


    private:
        double last_phase;

        bool prev_frame_silent;
        double prev_frame_freq;
};


#endif  // DIGISTRING_SYNTH_SINE_H
