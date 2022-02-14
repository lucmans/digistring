from scipy import signal

# import matplotlib.pyplot as plt


def main(args):
    if len(args) != 4:
        print("Error: Expected three arguments (output file relative to cwd, window length and dB attenuation)")
        print(f"Got {args[1:]}")
        return 1

    # Set parameters and calculate the window
    N = int(args[2])
    attenuation = float(args[3])
    win = signal.windows.chebwin(N + 1, attenuation, sym=False)[1:]

    # Normalize window such that the area of the window is 1
    max_val = 0
    for x in win:
        if x > max_val:
            max_val = x

    for idx, _ in enumerate(win):
        win[idx] = win[idx] / max_val

    # Write to a file which is read by Digistring
    with open(args[1], "w") as f:
        for n in win:
            f.write(str(n) + "\n")

    # # Show plot for debugging
    # plt.plot(win)
    # plt.show();

    return 0
