import parse_fraunhofer
# import parse_mdb_stem_synth

import gen_completions

import os


FRAUNHOFER = "fraunhofer"
MDB_STEM_SYNTH = "MDB-stem-synth"
DATASET_NAMES = [FRAUNHOFER, MDB_STEM_SYNTH]


def generate_report(dataset_name, dataset_annotations, digistring_results, report_filename):
    if dataset_name == FRAUNHOFER:
        # dataset_noteevents = parse_fraunhofer.parse_noteevents(dataset_annotations)
        print(f"{FRAUNHOFER} parser not yet implemented")
        exit(1)
    elif dataset_name == MDB_STEM_SYNTH:
        print(f"{MDB_STEM_SYNTH} parser not yet implemented")
        exit(1)
    else:
        print(f"No parser defined for {dataset_name}")
        exit(1)

    print(dataset_noteevents)


def print_dataset_names():
    print("Valid dataset names:")
    print("    " + " ".join(DATASET_NAMES))


def print_help():
    print("To generate performance report of Digistring on a dataset, run:")
    print("    ./generate_report <dataset name> <dataset annotations> <digistring results> <performance report output>")
    print_dataset_names()
    print("")
    print("To generate bash (tab) completions for dataset names, run:")
    print("    ./generate_report generate")
    print("This generates the file 'completions.sh'.")
    gen_completions.print_help()


def main(args):
    if len(args) == 2 and args[1] == "generate":
        gen_completions.generate(DATASET_NAMES)
        exit(0)

    if len(args) != 5:
        print(f"Invalid usage; expected 5 arguments, got {len(args)} instead.\n")
        print_help()
        exit(1)

    if args[1] not in DATASET_NAMES:
        print(f"{args[1]} is not a valid dataset name")
        print_dataset_names(DATASET_NAMES)
        exit(1)

    if not os.path.isfile(args[2]):
        print(f"'{args[2]}' is not a file")
        exit(1)

    if not os.path.isfile(args[3]):
        print(f"'{args[3]}' is not a file")
        exit(1)

    # TODO: Generate new filename
    if os.path.isfile(args[4]):
        print(f"'{args[4]}' already exists")
        exit(1)

    generate_report(args[1], args[2], args[3], args[4])