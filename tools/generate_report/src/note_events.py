from typing import Iterator
import copy


class Event:
    def __init__(self, pitch: int, onset: float, offset: float) -> None:
        self.pitch: int = pitch
        self.onset: float = onset
        self.offset: float = offset


    def __str__(self) -> str:
        return f"(pitch={self.pitch}, onset={self.onset}, offset={self.offset})"



class NoteEvents:
    def __init__(self) -> None:
        # List of dictionaries, where each dictionary is a note event
        # Each note event consists of a MIDI pitch number "pitch", an onset "onset" in second and an offset "offset" in seconds
        self.note_events: list[Event] = []

        self.sorted: bool = True


    # pitch is in MIDI note number and onset/offset in seconds
    def add_event(self, pitch: int, onset: float, offset: float) -> None:
        self.note_events.append(Event(pitch, onset, offset))
        self.sorted = False

    # Deep is not necessary here, but we do it anyway to prevent future bugs, as the overhead is minimal
    def copy_add_event(self, event: Event) -> None:
        self.note_events.append(copy.deepcopy(event))
        self.sorted = False

    def copy_add_events(self, events: list[Event]) -> None:
        self.note_events.extend(copy.deepcopy(events))
        self.sorted = False


    # TODO
    # def remove_event(self, del_event: Event) -> None:
    #     if self.sorted == False:
    #         self.sort_events()

    #     for event in self.note_events:
    #         if event.onset < del_event.onset:
    #             continue

    #         if event.onset > del_event.onset:
    #             break

    #         # Implicit 'event.onset == del_event.onset'
    #         if event.offset == del_event.offset and event.offset == del_event.offset:
    #             del()

    # def remove_events(self, del_events: list[Event] | NoteEvents) -> None:
    #     if self.sorted == False:
    #         self.sort_events()


    # Sort the events based on onset
    def sort_events(self) -> None:
        if self.sorted == True:
            return

        if len(self.note_events) < 2:
            self.sorted = True
            return

        # Sort on onset times
        self.note_events.sort(key=lambda event: event.onset)
        self.sorted = True


    # TODO: More efficient approach based on note_events being sorted on onset
    def is_monophonic(self) -> bool:
        # TODO: Needed?
        if self.sorted == False:
            self.sort_events()

        for li, levent in enumerate(self.note_events[:-1]):
            for ri, revent in enumerate(self.note_events[li+1:]):
                # If one event is fully to the left or right of another event, there is no overlap
                # If there is overlap, it is polyphonic
                if not (levent.onset >= revent.offset or levent.offset <= revent.onset):
                    return False

        return True

    def is_polyphonic(self) -> bool:
        # Sorted check in self.is_monophonic()
        return not self.is_monophonic()


    def total_note_time(self) -> float:
        total = 0.0
        for event in self.note_events:
            total += event.offset - event.onset

        return total


    ### Get methods ###
    # TODO: More efficient approach for get methods based on note_events being sorted on onset
    # Get methods always return a list (except explicit index getter, which should only be used internally)
    def get_events_containing_timepoint(self, timepoint: float) -> list[Event]:
        if self.sorted == False:
            self.sort_events()

        ret = []
        for event in self.note_events:
            if event.onset <= timepoint < event.offset:
                ret.append(event)

        return ret

    def get_events_containing_timeframe(self, start_time: float, stop_time: float) -> list[Event]:
        if self.sorted == False:
            self.sort_events()

        ret = []
        for event in self.note_events:
            # If event isn't to the left or right of the timeframe, it is contained in it
            if not (event.offset <= start_time or event.onset >= stop_time):
                ret.append(event)

        return ret

    # Events that are fully contained in the inclusive timeframe bounds
    def get_events_contained_timeframe(self, start_time: float, stop_time: float) -> list[Event]:
        if self.sorted == False:
            self.sort_events()

        ret = []
        for event in self.note_events:
            if event.onset >= start_time and event.offset <= stop_time:
                ret.append(event)

        return ret

    def get_events_containing_note_event(self, c_event: Event) -> list[Event]:
        if self.sorted == False:
            self.sort_events()

        ret = []
        for event in self.note_events:
            if event.onset >= c_event.onset and event.offset <= c_event.offset:
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


    def __len__(self) -> int:
        # TODO: Needed?
        if self.sorted == False:
            self.sort_events()

        return len(self.note_events)


    def __str__(self) -> str:
        if self.sorted == False:
            self.sort_events()

        out = "["

        if len(self.note_events) > 0:
            out += str(self.note_events[0])
        for event in self.note_events[1:]:
            out += ",\n" + str(event)

        out += "]"

        return out


    def __iter__(self) -> Iterator[Event]:
        if self.sorted == False:
            self.sort_events()

        return NoteEventIterator(self)


    # TODO: More efficient approach based on note_events being sorted on onset
    def __contains__(self, item: Event) -> bool:
        if self.sorted == False:
            self.sort_events()

        for event in self.note_events:
            if event.pitch == item.pitch and event.onset == item.onset and event.offset == item.offset:
                return True

            if event.onset > item.onset:
                break

        return False



class NoteEventIterator:
    def __init__(self, note_events: NoteEvents) -> None:
        self.index = 0
        self.note_events = note_events


    def __iter__(self) -> Iterator[Event]:
        return self


    def __next__(self) -> Event:
        if self.index >= len(self.note_events):
            raise StopIteration

        self.index += 1
        return self.note_events[self.index - 1]
