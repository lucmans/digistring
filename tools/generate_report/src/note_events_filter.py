import note_events as ne


# True positives
def correct_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    correct = ne.NoteEvents()

    for a_event in annotation_noteevents:
        c_events = digistring_noteevents.get_events_containing_timeframe(a_event.onset, a_event.offset)
        for p_event in c_events:
            if p_event.pitch == a_event.pitch:
                correct.copy_add_event(p_event)

    return correct


# False positives
def incorrect_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    correct = correct_notes(digistring_noteevents, annotation_noteevents)
    incorrect = ne.NoteEvents()

    for event in digistring_noteevents:
        if event not in correct:
            incorrect.copy_add_event(event)

    return incorrect


# False negatives
def missed_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    missed = ne.NoteEvents()

    for a_event in annotation_noteevents:
        c_events = digistring_noteevents.get_events_containing_timeframe(a_event.onset, a_event.offset)
        found = False
        for p_event in c_events:
            if p_event.pitch == a_event.pitch:
                found = True

        if found == False:
            missed.copy_add_event(a_event)

    return missed


# Even when only two of the three sets are needed, this is more efficient
def correct_incorrect_missed_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    correct = ne.NoteEvents()    # True positives
    incorrect = ne.NoteEvents()  # False positives
    missed = ne.NoteEvents()     # False negatives

    # First do correct and missed as they can be found in one pass
    for a_event in annotation_noteevents:
        c_events = digistring_noteevents.get_events_containing_timeframe(a_event.onset, a_event.offset)
        found = False
        for p_event in c_events:
            if p_event.pitch == a_event.pitch:
                correct.copy_add_event(p_event)
                found = True

        if found == False:
            missed.copy_add_event(a_event)

    # All the Digistring note events which weren't correct are incorrect
    for event in digistring_noteevents:
        if event not in correct:
            incorrect.copy_add_event(event)

    return correct, incorrect, missed


def filter_transient_errors(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    correct, incorrect, _ = correct_incorrect_missed_notes(digistring_noteevents, annotation_noteevents)
    filtered = ne.NoteEvents()

    for event in incorrect:
        pass

    print("Warning: Transient error filtering not yet implemented (returning all incorrect note events)")
    filtered = incorrect

    return filtered
