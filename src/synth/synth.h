#ifndef DIGISTRING_SYNTH_SYNTH_H
#define DIGISTRING_SYNTH_SYNTH_H


#include "note.h"


/* When adding a new synth, don't forget to include the file in synths.h */

// Different synth types
enum class Synths {
    sine
};


class Synth {
    public:
        Synth();
        virtual ~Synth();

        virtual void synthesize(const NoteEvents &notes, float *const synth_buffer, const int n_samples) = 0;


    protected:
        //
};


#endif  // DIGISTRING_SYNTH_SYNTH_H
