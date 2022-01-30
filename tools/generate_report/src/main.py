#!/bin/python3

import gen_completions

import sys, errno


DATASET_NAMES = ["fraunhofer", "MDB-stem-synth"]

def print_dataset_names():
    print("Valid dataset names:")
    print("    " + " ".join(DATASET_NAMES))

def print_help():
    print("To generate performance report of Digistring on a dataset, run:")
    print(f"    ./generate_report <dataset name> <dataset annotations> <digistring results> <performance report output>")
    print_dataset_names()
    print("")
    print("To generate bash (tab) completions for dataset names, run:")
    print(f"    ./generate_report generate")
    print("This generates the file 'completions.sh'.")
    gen_completions.print_help()


if __name__ == "__main__":
    if len(sys.argv) == 2 and sys.argv[1] == "generate":
        gen_completions.generate(DATASET_NAMES)
        exit(0)

    if len(sys.argv) != 5:
        print("Invalid usage\n")
        print_help()
        exit(1)

    if sys.argv[1] not in DATASET_NAMES:
        print(f"{sys.argv[1]} is not a valid dataset name")
        print_dataset_names(DATASET_NAMES)
        exit(1)

    print("TODO")
