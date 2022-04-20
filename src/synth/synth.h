#ifndef DIGISTRING_SYNTH_SYNTH_H
#define DIGISTRING_SYNTH_SYNTH_H


#include "note.h"

#include <map>
#include <string>


/* When adding a new synth, don't forget to include the file in synths.h and add it to the factory */

// Different synth types
enum class Synths {
    square, sine, sine_amped
};

// For selecting a synth using CLI args
const std::map<const std::string, const Synths> parse_synth_string = {
    {"square", Synths::square},
    {"sine", Synths::sine},
    {"sine_amped", Synths::sine_amped}
};

const std::map<const Synths, const std::string> synth_description = {
    {Synths::square, "Simple square wave synth"},
    {Synths::sine, "Simple sine wave synth"},
    {Synths::sine_amped, "Sine wave synth with variable amplitude"}
};


class Synth {
    public:
        Synth();
        virtual ~Synth();

        virtual void synthesize(const NoteEvents &notes, float *const synth_buffer, const int n_samples) = 0;

        void reset_max_amp();
        void set_max_amp(const double _max_amp);


    protected:
        double max_amp;
};


#endif  // DIGISTRING_SYNTH_SYNTH_H
