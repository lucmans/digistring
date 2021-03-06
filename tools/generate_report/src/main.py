import parse_digistring
import parse_fraunhofer
# import parse_mdb_stem_synth

import note_events_filter
import note_events_grapher

import gen_completions

import os
import signal
import sys


FRAUNHOFER = "fraunhofer"
MDB_STEM_SYNTH = "MDB-stem-synth"
DATASET_NAMES = [FRAUNHOFER, MDB_STEM_SYNTH]


def signal_handler(signal, frame):
    print(f"\nReceived signal {signal}, quitting...")
    exit(0)


def generate_report(dataset_name: str, dataset_annotations: str, digistring_results: str, report_filename: str) -> None:
    # Parse dataset annotations
    if dataset_name == FRAUNHOFER:
        dataset_noteevents = parse_fraunhofer.parse_noteevents(dataset_annotations)
    elif dataset_name == MDB_STEM_SYNTH:
        print(f"{MDB_STEM_SYNTH} parser not yet implemented")
        exit(1)
    else:
        print(f"No parser defined for {dataset_name}")
        exit(1)

    # Parse Digistring results
    digistring_noteevents = parse_digistring.parse_noteevents(digistring_results)
    if len(digistring_noteevents) == 0:
        print("Error: No note events in file")
        exit(1)
    print(f"Total Digistring events: {len(digistring_noteevents)}")

    # Correct = true positives
    # Incorrect = false positives
    # Missed = false negatives
    digistring_correct, digistring_incorrect, digistring_missed = note_events_filter.correct_incorrect_missed_notes(digistring_noteevents, dataset_noteevents)
    print(f"  - Correct: {len(digistring_correct)}")
    print(f"  - Incorrect: {len(digistring_incorrect)}")
    print(f"  - Missed: {len(digistring_missed)}")
    print()

    precision = len(digistring_correct) / (len(digistring_correct) + len(digistring_incorrect))
    recall = len(digistring_correct) / (len(digistring_correct) + len(digistring_missed))
    print("Performance measures:")
    print(f"  - Precision: {precision:.4f}")
    print(f"  - Recall: {recall:.4f}")
    if precision + recall == 0.0:
        f1 = 0.0
    else:
        f1 = 2.0 * ((precision * recall) / (precision + recall))
    print(f"  - F1: {f1:.4f}")
    print()

    total_digistring_s = digistring_noteevents.total_note_time()
    total_dataset_s = dataset_noteevents.total_note_time()
    print("Total note time:")
    print(f"  - Digistring: {total_digistring_s:.3f} s")
    print(f"  - Annotations: {total_dataset_s:.3f} s")
    print()

    correct_s, missed_s, overshot_s = note_events_filter.seconds_correct_missed_overshot(digistring_noteevents, dataset_noteevents)
    print("Total time estimated...")
    print(f"  - Correct: {correct_s:.3f} s")
    print(f"  - Missed: {missed_s:.3f} s")
    print(f"  - Overshot: {overshot_s:.3f} s")
    print()

    digistring_transient, digistring_tr_incorrect = note_events_filter.transient_errors(digistring_noteevents, dataset_noteevents)
    print(f"Transient errors: {len(digistring_transient)}")
    print(f"Transient inc: {len(digistring_tr_incorrect)}")
    print()

    # Plot the note events (note that this call blocks until the UI is closed)
    note_events_grapher.graph([
            # note_events_grapher.make_plot("Transient", digistring_transient, "C5"),
            note_events_grapher.make_plot("Transient", digistring_transient, "C5"),
            note_events_grapher.make_plot("Tr inc", digistring_tr_incorrect, "C6"),
            note_events_grapher.make_plot("Correct", digistring_correct, "C2"),
            # note_events_grapher.make_plot("Digistring", digistring_noteevents, "C0"),
            note_events_grapher.make_plot("Annotations", dataset_noteevents, "C1"),
            note_events_grapher.make_plot("Incorrect", digistring_incorrect, "C3"),
            note_events_grapher.make_plot("Missed", digistring_missed, "C4"),
        ])


def print_dataset_names() -> None:
    print("Valid dataset names:")
    print("    " + " ".join(DATASET_NAMES))


def print_help() -> None:
    print("To generate performance report of Digistring on a dataset, run:")
    print("    ./generate_report <dataset name> <dataset annotations> <digistring results> <performance report output>")
    print_dataset_names()
    print("")
    print("To generate bash (tab) completions for dataset names, run:")
    print("    ./generate_report generate")
    print("This generates the file 'completions.sh'.")
    gen_completions.print_help()


def main(args: list[str]) -> int:
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)

    # DEBUG
    if len(args) == 2:
        if args[1] == "1":
            generate_report("fraunhofer", "lick.xml", "lick.json", "lick.output")
        elif args[1] == "2":
            generate_report("fraunhofer", "dataset/dataset2/annotation/AR_A_fret_0-20.xml", "AR_A_fret_0-20.json", "AR_A_fret_0-20.output")
        elif args[1] == "3":
            generate_report("fraunhofer", "lick_missed.xml", "lick.json", "lick.output")
        elif args[1] == "4":
            generate_report("fraunhofer", "lick.xml", "lick_dolph.json", "lick.output")

        elif args[1] == "generate":
            gen_completions.generate(DATASET_NAMES)

        else:
            print(f"'{args[1]}' not defined; doing nothing...")
            print_help()

        exit(0)

    if len(args) == 2 and args[1] == "generate":
        gen_completions.generate(DATASET_NAMES)
        return 0

    if len(args) != 5:
        print(f"Invalid usage; expected 5 arguments, got {len(args)} instead.\n")
        print_help()
        return 1

    if args[1] not in DATASET_NAMES:
        print(f"{args[1]} is not a valid dataset name")
        print_dataset_names()
        return 1

    if not os.path.isfile(args[2]):
        print(f"'{args[2]}' is not a file")
        return 1

    if not os.path.isfile(args[3]):
        print(f"'{args[3]}' is not a file")
        return 1

    # TODO: Generate new filename
    if os.path.isfile(args[4]):
        print(f"'{args[4]}' already exists")
        return 1

    generate_report(args[1], args[2], args[3], args[4])

    return 0
