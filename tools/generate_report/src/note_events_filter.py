import note_events as ne

from typing import Union


def correct_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    correct = ne.NoteEvents()

    for a_event in annotation_noteevents:
        c_events = digistring_noteevents.get_events_containing_timeframe(a_event["onset"], a_event["offset"])
        for p_event in c_events:
            if p_event["pitch"] == a_event["pitch"]:
                correct.copy_add_event(p_event)

    return correct


def incorrect_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents, return_correct: bool = False) -> Union[ne.NoteEvents, tuple[ne.NoteEvents, ne.NoteEvents]]:
    correct = correct_notes(digistring_noteevents, annotation_noteevents)
    incorrect = ne.NoteEvents()

    for event in digistring_noteevents:
        if event not in correct:
            incorrect.copy_add_event(event)

    # Often both sets are needed; this saves many calculations are these function are computationally expensive
    if return_correct:
        return incorrect, correct
    else:
        return incorrect


def filter_transient_errors(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    incorrect, correct = incorrect_notes(digistring_noteevents, annotation_noteevents, return_correct=True)
    filtered = ne.NoteEvents()

    for event in incorrect:
        pass

    print("Warning: Transient error filtering not yet implemented (returning all incorrect note events)")
    return incorrect

    return filtered
