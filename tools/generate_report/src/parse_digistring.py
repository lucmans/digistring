import note_events as ne

import json


def parse_noteevents(digistring_results: str) -> ne.NoteEvents:
    # Read and parse the JSON file
    json_events = None
    with open(digistring_results) as file:
        json_events = json.load(file)

    # Extract the note events
    note_events = ne.NoteEvents()

    # First note explicit
    if len(json_events["note events"]) > 0:
        start_note_event = json_events["note events"][0]

    for event in json_events["note events"][1:]:
        # No new note, so continue
        if event["midi_number"] == start_note_event["midi_number"]:
            continue

        # Don't output silence as note event
        if start_note_event["midi_number"] != None:
            note_events.add_event(start_note_event["midi_number"], float(start_note_event["t (s)"]), float(event["t (s)"]))

        # Save new note
        start_note_event = event

    return note_events
