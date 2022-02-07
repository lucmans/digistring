import numpy as np
import matplotlib.pyplot as plt


SEPARATION = 0.1
LINE_WIDTH = 3

LEGEND_LOC = "upper left"

FONT_SIZE = {"label": 20,
             "ticks": 15,
             "legend": 18}


def plot_digistring(event, label=None):
    plt.plot([event["onset"], event["offset"]], [event["pitch"] + SEPARATION, event["pitch"] + SEPARATION],
            color="C0",
            linewidth=LINE_WIDTH,
            label=label)

def plot_annotations(event, label=None):
    plt.plot([event["onset"], event["offset"]], [event["pitch"] - SEPARATION, event["pitch"] - SEPARATION],
            color="C1",
            linewidth=LINE_WIDTH,
            label=label)


def graph(digistring_noteevents, annotations_noteevents):
    # One separate for one legend entry
    if len(digistring_noteevents) > 0:
        plot_digistring(digistring_noteevents[0], "Digistring")
    for event in digistring_noteevents[1:]:
        plot_digistring(event)

    if len(annotations_noteevents) > 0:
        plot_annotations(annotations_noteevents[0], "Annotations")
    for event in annotations_noteevents[1:]:
        plot_annotations(event)

    plt.xlabel("time (s)", fontsize=FONT_SIZE["label"])
    plt.xticks(fontsize=FONT_SIZE["ticks"])
    plt.ylabel("pitch", fontsize=FONT_SIZE["label"])
    plt.yticks(fontsize=FONT_SIZE["ticks"])

    plt.legend(loc=LEGEND_LOC, fontsize=FONT_SIZE["legend"])

    plt.show();
