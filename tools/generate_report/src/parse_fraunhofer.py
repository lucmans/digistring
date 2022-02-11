import note_events as ne

import xml.etree.ElementTree as et


def parse_noteevents(annotations_filename: str) -> ne.NoteEvents:
    annotations = et.parse(annotations_filename)
    root = annotations.getroot()

    # Sanity check
    xlm_events = root.findall("transcription")
    if len(xlm_events) != 1:
        raise RuntimeError(f"Found {len(xlm_events)} transcription tags instead of 1")

    # Extract the note events
    note_events = ne.NoteEvents()
    for event in xlm_events[0]:
        res = event.findall("pitch")
        if res is None:
            RuntimeError(f"No pitch in note event #{len(note_events) + 1}")
        elif len(res) != 1:
            RuntimeError(f"Multi pitch tags in note event #{len(note_events) + 1}")
        pitch = int(res[0].text)

        res = event.findall("onsetSec")
        if res is None:
            RuntimeError(f"No onset in note event #{len(note_events) + 1}")
        elif len(res) != 1:
            RuntimeError(f"Multi onset tags in note event #{len(note_events) + 1}")
        onset = float(res[0].text)

        res = event.findall("offsetSec")
        if res is None:
            RuntimeError(f"No offset in note event #{len(note_events) + 1}")
        elif len(res) != 1:
            RuntimeError(f"Multi offset tags in note event #{len(note_events) + 1}")
        offset = float(res[0].text)

        note_events.add_event(pitch, onset, offset)

    return note_events
