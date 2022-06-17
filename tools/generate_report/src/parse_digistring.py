import note_events as ne

import json


class OngoingNote:
    # onset and duration in number of samples
    def __init__(self, onset: int, duration: int) -> None:
        self.onset: int = onset
        self.duration: int = duration


def parse_noteevents(digistring_results: str) -> ne.NoteEvents:
    note_events = ne.NoteEvents()

    # Read and parse the JSON file
    json_events = None
    try:
        with open(digistring_results) as file:
            json_events = json.load(file)
    except json.JSONDecodeError:
        print(f"'{digistring_results}' is not a valid JSON file")

    # JSON -> NoteEvents
    sample_rate = json_events["Sample rate (Hz)"]
    ongoing_notes = {}
    for event in json_events["note events"]:
        # No notes in frame
        if event["midi_number"] == None:
            # Flush all ongoing notes
            for on_midi, on_on in ongoing_notes.items():
                note_events.add_event(on_midi, on_on.onset / sample_rate, (on_on.onset + on_on.duration) / sample_rate)
            ongoing_notes = {}

        # If note was in previous frame...
        elif event["midi_number"] in ongoing_notes:
            matching_ongoing = ongoing_notes[event["midi_number"]]
            # ...and offset perfectly aligns with onset
            if matching_ongoing.onset + matching_ongoing.duration == event["note_start (samples)"]:
                # Extent the note event
                matching_ongoing.duration += event["note_duration (samples)"]
            elif matching_ongoing.onset + matching_ongoing.duration > event["note_start (samples)"]:
                # If the new note starts before the old one ends, invalid note overlap
                print("Warning: Overlapping note events; merging them into one...")
                print(f"Merged note {event['midi_number']} at [{matching_ongoing.onset / sample_rate}, {(matching_ongoing.onset + matching_ongoing.duration) / sample_rate}]"
                        + f" and [{event['note_start (s)']}, {event['note_start (s)'] + event['note_duration (s)']}]", end="")
                matching_ongoing.duration += event["note_duration (samples)"] - ((matching_ongoing.onset + matching_ongoing.duration) - event["note_start (samples)"])
                print(f" merged to [{matching_ongoing.onset / sample_rate}, {matching_ongoing.duration / sample_rate}]")
            else:
                # Space between notes, so flush old and start new
                note_events.add_event(event["midi_number"], matching_ongoing.onset / sample_rate, (matching_ongoing.onset + matching_ongoing.duration) / sample_rate)
                ongoing_notes[event["midi_number"]] = OngoingNote(event["note_start (samples)"], event["note_duration (samples)"])

        # Note is not in ongoing_notes
        else:
            # So add it
            ongoing_notes[event["midi_number"]] = OngoingNote(event["note_start (samples)"], event["note_duration (samples)"])

    # Flush all notes that were still ongoing at the end
    for on_midi, on_on in ongoing_notes.items():
        note_events.add_event(on_midi, on_on.onset / sample_rate, (on_on.onset + on_on.duration) / sample_rate)

    return note_events
