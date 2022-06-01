import sys

import matplotlib.pyplot as plt


INVERT_ORDER = True


def main(args):
    # Parse CLI args
    ask_titles = False
    if len(args) < 2 or len(args) > 3:
        print("Error: Expected one or two arguments. First is the performance file and optionally a second argument 'ask_labels'.")
        return 1

    if len(args) == 3:
        if args[2] == "ask_labels":
            ask_titles = True
        else:
            print("Error: Invalid second argument\nExpected 'ask_labels' or nothing")
            return 1

    # Parse performance file
    data = []
    with open(args[1]) as f:
        for line in f:
            title = line.strip()
            line = next(f)
            durations = list(map(float, line.strip().split(" ")))
            data.append([title, durations])
            next(f)


    raw_titles, durations = zip(*data)

    titles = [""]
    if ask_titles:
        for title in raw_titles:
            new_title = input(f"Enter short name for '{title}':\n")
            titles.append(new_title)
    else:
        titles.extend(list(raw_titles))

    fig, ax = plt.subplots()
    if INVERT_ORDER:
        ax.violinplot(durations, vert=False, positions=list(range(1, len(titles)))[::-1])
    else:
        ax.violinplot(durations, vert=False)

    ax.set_title("Duration of different Digistring components", fontsize=30)

    ax.set_xlabel("Duration (ms)")#, fontsize=30)

    ax.set_yticks(list(range(len(titles))))
    ax.set_yticklabels(titles)

    plt.show()

    return 0


if __name__ == "__main__":
    exit(main(sys.argv))
