import note_events as ne

import numpy as np
import matplotlib.pyplot as plt

from typing import Optional, TypedDict


SEPARATION = 0.2
LINE_WIDTH = 3

LEGEND_LOC = "upper left"

FONT_SIZE = {"label": 20,
             "ticks": 15,
             "legend": 18}


class NoteEventPlot(TypedDict):
    events: ne.NoteEvents
    color: str
    label: str


def make_plot(label: str, events: ne.NoteEvents, color: str) -> NoteEventPlot:
    return {"events": events, "color": color, "label": label}


def plot_noteevent(event: ne.Event, draw_y_offset: float, color: str, label: Optional[str] = None) -> None:
    plt.plot([event["onset"], event["offset"]], [event["pitch"] + draw_y_offset, event["pitch"] + draw_y_offset],
             color=color,
             linewidth=LINE_WIDTH,
             label=label)


def plot_noteevents(plot: NoteEventPlot, draw_y_offset: float) -> None:
    if len(plot["events"]) == 0:
        print(f"Warning: No note events in {plot['label']}")
        return

    plot_noteevent(plot["events"][0], draw_y_offset, plot["color"], plot["label"])
    for event in plot["events"][1:]:
        plot_noteevent(event, draw_y_offset, plot["color"])


# Note that this call blocks on plt.show() until the user closes the plot
def graph(noteevent_plots: list[NoteEventPlot]) -> None:
    n_plots = len(noteevent_plots)
    offsets = []
    for i in range(n_plots):
        offsets.append((((n_plots - 1) - (2 * i)) * SEPARATION) / 2)

    for idx, plot in enumerate(noteevent_plots):
        plot_noteevents(plot, offsets[idx])

    plt.xlabel("time (s)", fontsize=FONT_SIZE["label"])
    plt.xticks(fontsize=FONT_SIZE["ticks"])
    plt.ylabel("pitch", fontsize=FONT_SIZE["label"])
    plt.yticks(fontsize=FONT_SIZE["ticks"])

    plt.legend(loc=LEGEND_LOC, fontsize=FONT_SIZE["legend"])

    plt.show()


# def graph(digistring_noteevents: ne.NoteEvents, annotations_noteevents: ne.NoteEvents) -> None:
#     # One separate for one legend entry
#     if len(digistring_noteevents) > 0:
#         plot_digistring(digistring_noteevents[0], "Digistring")
#     for event in digistring_noteevents[1:]:
#         plot_digistring(event)

#     if len(annotations_noteevents) > 0:
#         plot_annotations(annotations_noteevents[0], "Annotations")
#     for event in annotations_noteevents[1:]:
#         plot_annotations(event)

#     plt.xlabel("time (s)", fontsize=FONT_SIZE["label"])
#     plt.xticks(fontsize=FONT_SIZE["ticks"])
#     plt.ylabel("pitch", fontsize=FONT_SIZE["label"])
#     plt.yticks(fontsize=FONT_SIZE["ticks"])

#     plt.legend(loc=LEGEND_LOC, fontsize=FONT_SIZE["legend"])

#     plt.show();
