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
def correct_incorrect_missed_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> tuple[ne.NoteEvents, ne.NoteEvents, ne.NoteEvents]:
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


def transient_errors(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    correct, incorrect, _ = correct_incorrect_missed_notes(digistring_noteevents, annotation_noteevents)
    t_errors = ne.NoteEvents()

    for a_event in annotation_noteevents:
        t_errors.copy_add_events(incorrect.get_events_containing_timepoint(a_event.onset))

    return t_errors


def seconds_correct_missed_overshot(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> tuple[float, float]:
    correct = 0.0
    missed = 0.0
    overshot = 0.0

    # Same as correct filter, but instead of adding correct notes to a set, add the missed and overshot seconds to totals
    for a_event in annotation_noteevents:
        c_events = digistring_noteevents.get_events_containing_timeframe(a_event.onset, a_event.offset)
        for p_event in c_events:
            if p_event.pitch == a_event.pitch:
                correct += p_event.offset - p_event.onset

                if p_event.onset > a_event.onset:
                    missed += p_event.onset - a_event.onset
                elif p_event.onset < a_event.onset:
                    overshot += a_event.onset - p_event.onset

                if p_event.offset < a_event.offset:
                    missed += a_event.offset - p_event.offset
                elif p_event.offset > a_event.offset:
                    overshot += p_event.offset - a_event.offset

    return correct, missed, overshot
