import note_events as ne

import xml.etree.ElementTree as et


def parse_noteevents(annotations_filename: str) -> ne.NoteEvents:
# def parse_noteevents(annotations_filename):
    annotations = et.parse(annotations_filename)
    root = annotations.getroot()

    # Sanity check
    xlm_events = root.findall("transcription")
    if len(xlm_events) != 1:
        raise RuntimeError(f"Found {len(xlm_events)} transcription tags instead of 1")

    # Extract the note events
    note_events = ne.NoteEvents()
    for event in xlm_events[0]:
        # if event.find("pitch").text != None:
        #     pitch = int(event.find("pitch").text)
        # else:
        #     raise RuntimeError(f"Found {len(xlm_events)} transcription tags instead of 1")

        pitch = int(event.find("pitch").text)
        onset = float(event.find("onsetSec").text)
        offset = float(event.find("offsetSec").text)
        note_events.add_event(pitch, onset, offset)

    return note_events
