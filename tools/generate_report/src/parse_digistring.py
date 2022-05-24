import note_events as ne

import json


def parse_noteevents(digistring_results: str) -> ne.NoteEvents:
    print("Digistring currently uses a newer JSON output formatting")
    print("If you still have old output files, remove this exit condition in 'src/parse_digistring.py'")
    exit(-1)

    # Read and parse the JSON file
    with open(digistring_results) as file:
        json_events = json.load(file)

    # Extract the note events
    note_events = ne.NoteEvents()

    # First note explicit, as there is no previous event
    if len(json_events["note events"]) > 0:
        start_note_event = json_events["note events"][0]

    for event in json_events["note events"][1:]:
        # No new note, so continue
        if event["midi_number"] == start_note_event["midi_number"]:
            continue

        # Don't output silence as note event
        if start_note_event["midi_number"] is not None:
            note_events.add_event(start_note_event["midi_number"], float(start_note_event["t (s)"]), float(event["t (s)"]))

        # Save new note
        start_note_event = event

    if json_events["note events"][-1]["midi_number"] is not None:
        print("Error: Last event should be a silent one (null in all fields)")
        exit(1)

    return note_events
