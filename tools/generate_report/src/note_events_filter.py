import note_events as ne


def correct_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    correct = ne.NoteEvents()

    for a_event in annotation_noteevents:
        print(f"a_event {a_event}")
        c_events = digistring_noteevents.get_events_containing_timeframe(a_event["onset"], a_event["offset"])
        for p_event in c_events:
            if p_event["pitch"] == a_event["pitch"]:
                correct.copy_add_event(p_event)

    return correct


def incorrect_notes(digistring_noteevents: ne.NoteEvents, annotation_noteevents: ne.NoteEvents) -> ne.NoteEvents:
    incorrect = ne.NoteEvents()

    correct = correct_notes(digistring_noteevents, annotation_noteevents)
    # TODO: incorrect = digistring_noteevents - correct
    print("Warning: Incorrect notes filter not yet implemented")

    return incorrect

