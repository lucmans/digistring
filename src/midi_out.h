#ifndef DIGISTIRNG_MIDI_OUT_H
#define DIGISTIRNG_MIDI_OUT_H


#include "note.h"

#include "config/synth.h"

#ifdef NO_ALSA_MIDI
    typedef int snd_rawmidi_t;
#else
    #include <alsa/asoundlib.h>
#endif

#include <set>


class MidiOut {
    public:
        MidiOut();
        ~MidiOut();

        void reset_loudest_note();
        void send(const NoteEvents &estimated_events);


    private:
        snd_rawmidi_t *midi_output_dev;

        std::set<int> prev_frame_notes;
        double loudest_note;
};


#endif  // DIGISTIRNG_MIDI_OUT_H
