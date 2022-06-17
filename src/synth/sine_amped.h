#ifndef DIGISTRING_SYNTH_SINE_AMPED_H
#define DIGISTRING_SYNTH_SINE_AMPED_H


#include "synth.h"

#include "note.h"


class SineAmped : public Synth {
    public:
        SineAmped();
        ~SineAmped() override;

        void synthesize(const NoteEvents &note_events, float *const synth_buffer, const int n_samples, const double volume = 1.0);


    private:
        double last_phase;

        bool prev_frame_silent;
        double prev_frame_freq;
        double prev_frame_amp;
};


#endif  // DIGISTRING_SYNTH_SINE_AMPED_H
