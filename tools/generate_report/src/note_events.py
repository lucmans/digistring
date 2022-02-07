class NoteEventIterator:
    def __init__(self, note_events):
        self.index = 0
        self.note_events = note_events


    def __next__(self):
        if self.index >= len(self.note_events):
            raise StopIteration

        self.index += 1
        return self.note_events[self.index - 1]



class NoteEvents:
    def __init__(self):
        # List of dictionaries, where each dictionary is a note event
        # Each note event consists of a MIDI pitch number "pitch", an onset "onset" in second and an offset "offset" in seconds
        self.note_events = []

        self.sorted = False


    # pitch is in MIDI note number and onset/offset in seconds
    def add_event(self, pitch, onset, offset):
        self.note_events.append({"pitch": pitch, "onset": onset, "offset": offset})
        self.sorted = False


    # Sort the events
    def sort_events(self):
        if self.sorted == True:
            return

        # Sort on onset times
        self.note_events.sort(key=lambda event: event["onset"])
        self.sorted = True


    # TODO: More efficient approach
    def is_monophonic(self):
        # TODO: Needed?
        if self.sorted == False:
            self.sort_events()

        for li, levent in enumerate(self.note_events[:-1]):
            for ri, revent in enumerate(self.note_events[li+1:]):
                # If one event is fully to the left or right of another event, there is no overlap
                # If there is overlap, it is polyphonic
                if not (levent["onset"] >= revent["offset"] or levent["offset"] <= revent["onset"]):
                    return False

        return True

    def is_polyphonic(self):
        # Sorted check in self.is_monophonic()
        return not self.is_monophonic()


    ### Get methods ###
    # Get methods always return a list (except explicit index getter, which should only be used internally)
    def get_events_containing_timepoint(self, timepoint):
        if self.sorted == False:
            self.sort_events()

        ret = []
        for event in self.note_events:
            if timepoint >= event["onset"] and timepoint < event["offset"]:
                ret.append(event)

        return ret

    def get_events_containing_timeframe(self, start_time, stop_time):
        if self.sorted == False:
            self.sort_events()

        ret = []
        for event in self.note_events:
            # If event isn't to the left or right of the timeframe, it is contained in it
            if not (event["offset"] < start_time or event["onset"] > stop_time):
                ret.append(event)

        return ret

    # Events that are fully contained in the inclusive timeframe bounds
    def get_events_contained_timeframe(self, start_time, stop_time):
        if self.sorted == False:
            self.sort_events()

        ret = []
        for event in self.note_events:
            if start_time >= event["onset"] and stop_time <= event["offset"]:
                ret.append(event)

        return ret


    def __get_event_index(self, index):
        if self.sorted == False:
            self.sort_events()

        return self.note_events[index]

    # Overload the subscript operator (for NoteEventIterator)
    def __getitem__(self, key):
        # self.sorted check in get method
        return self.__get_event_index(key)


    def __len__(self):
        # TODO: Needed?
        if self.sorted == False:
            self.sort_events()

        return len(self.note_events)


    def __str__(self):
        if self.sorted == False:
            self.sort_events()

        out = "["

        if len(self.note_events) > 0:
            out += str(self.note_events[0])
        for event in self.note_events[1:]:
            out += ",\n" + str(event)

        out += "]"

        return out


    def __iter__(self):
        if self.sorted == False:
            self.sort_events()

        return NoteEventIterator(self)
