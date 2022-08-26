#ifndef DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_NOTE_SELECTORS_H
#define DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_NOTE_SELECTORS_H


#include "note.h"


void get_loudest_peak(NoteSet &out_notes, const NoteSet &peaks);
void get_lowest_peak(NoteSet &out_notes, const NoteSet &peaks);
void get_most_overtones(NoteSet &out_notes, const NoteSet &peaks);
void get_most_overtones(NoteSet &out_notes, const NoteSet &peaks, NoteSet &out_note_peaks);
void get_most_overtone_power(NoteSet &out_notes, const NoteSet &peaks);


#endif  // DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_NOTE_SELECTORS_H
