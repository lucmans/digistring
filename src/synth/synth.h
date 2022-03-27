#ifndef DIGISTRING_SYNTH_SYNTH_H
#define DIGISTRING_SYNTH_SYNTH_H


#include "note.h"

#include <map>
#include <string>


/* When adding a new synth, don't forget to include the file in synths.h */

// Different synth types
enum class Synths {
    sine, square
};

// For selecting a synth using CLI args
const std::map<std::string, Synths> parse_synth_string = {{"sine", Synths::sine},
                                                          {"square", Synths::square}};


class Synth {
    public:
        Synth();
        virtual ~Synth();

        virtual void synthesize(const NoteEvents &notes, float *const synth_buffer, const int n_samples) = 0;


    protected:
        //
};


#endif  // DIGISTRING_SYNTH_SYNTH_H
