#include "midi_out.h"

#ifdef NO_ALSA_MIDI


#include "error.h"

MidiOut::MidiOut() {
    warning("ALSA support is not compiled in; MidiOut object does nothing...");
    hint("Compile Digistring with ALSA MIDI support (see 'COMPILE_CONFIG' in the makefile)");
}

MidiOut::~MidiOut() {}

void MidiOut::reset_loudest_note() {}

void MidiOut::send(const NoteEvents &estimated_events) {
    return;
    // Prevent warning
    int i = 0;
    for(const auto &event : estimated_events) i += event.offset;
}


#else


#include "note.h"
#include "error.h"

#include <alsa/asoundlib.h>

#include <utility>  //std::move()
#include <cstdlib>  // EXIT_FAILURE


const uint8_t ALL_NOTES_OFF_EVENT[3] = {0xB0, 123, 0};
uint8_t NOTE_ON_EVENT[3] = {0x90, 0, 64};  // NOTE_ON_EVENTs[1] holds MIDI note number, [2] hold volume
uint8_t NOTE_OFF_EVENT[3] = {0x80, 0, 0};  // NOTE_OFF_EVENTs[1] holds MIDI note number


MidiOut::MidiOut() {
    int ret = snd_rawmidi_open(NULL, &midi_output_dev, "virtual", O_WRONLY);
    if(ret < 0) {
        error("Alsa failed to create raw MIDI output");
        exit(EXIT_FAILURE);
    }

    loudest_note = 0.1;
}

MidiOut::~MidiOut() {
    snd_rawmidi_write(midi_output_dev, ALL_NOTES_OFF_EVENT, 3);  // Stop all notes
    int ret = snd_rawmidi_close(midi_output_dev);
    if(ret < 0) {
        error("Alsa failed to close raw MIDI output");
        exit(EXIT_FAILURE);
    }
}


void MidiOut::reset_loudest_note() {
    loudest_note = 0.1;
}


void MidiOut::send(const NoteEvents &estimated_events) {
    std::set<int> frame_notes;
    for(const auto &event : estimated_events) {
        frame_notes.insert(event.note.midi_number);

        if(event.note.amp > loudest_note)
            loudest_note = event.note.amp;

        // Turn on started notes
        if(prev_frame_notes.count(event.note.midi_number) == 0) {
            NOTE_ON_EVENT[1] = event.note.midi_number;
            NOTE_ON_EVENT[2] = (log2(event.note.amp) / log2(loudest_note)) * MAX_VOLUME;
            snd_rawmidi_write(midi_output_dev, NOTE_ON_EVENT, 3);
        }
    }

    // Turn off stopped notes
    for(const auto &prev_note : prev_frame_notes) {
        if(frame_notes.count(prev_note) == 0) {
            NOTE_OFF_EVENT[1] = prev_note;
            snd_rawmidi_write(midi_output_dev, NOTE_OFF_EVENT, 3);
        }
    }

    // Swap current events to prev events
    prev_frame_notes = std::move(frame_notes);

    // // Trigger notes every frame
    // snd_rawmidi_write(midi_output_dev, ALL_NOTES_OFF_EVENT, 3);  // Stop notes from previous frame
    // for(const auto &event : estimated_events) {
    //     NOTE_ON_EVENT[1] = event.note.midi_number;
    //     NOTE_ON_EVENT[2] = (event.note.amp / loudest_note) * 127.0;
    //     snd_rawmidi_write(midi_output_dev, NOTE_ON_EVENT, 3);
    // }
}


#endif
