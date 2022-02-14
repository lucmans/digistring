from scipy import signal
# import matplotlib.pyplot as plt

import warnings


def main(args):
    if len(args) != 4:
        print("\nError: Expected three arguments (output file relative to cwd, window length and dB attenuation)")
        print(f"Got {args[1:]}\n")
        return 1

    # Create error when warning is given by chebwin() function
    warnings.filterwarnings("error")

    # Set parameters and calculate the window
    N = int(args[2])
    attenuation = float(args[3])
    win = None
    try:
        win = signal.windows.chebwin(N + 1, attenuation, sym=False)[1:]
    except UserWarning as e:
        print(f"\nError: SciPy warning when creating Dolph Chebyshev window:\n{e}\n")
        return 2

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
