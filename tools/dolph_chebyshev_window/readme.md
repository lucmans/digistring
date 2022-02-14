`doplh_chebyshev_window` calculates the Dolph Chebyshev window and outputs it to the specified file.  
Digistring calls this script and reads the output file to obtain the window.


# Build instructions
## Requirements
Python >=3.10 and SciPy >=1.7.3  
Furthermore, `python3` should be present in the environment variable PATH of the environment running Digistring.

On Arch Linux:  
`sudo pacman -S python3 python-scipy`


# Usage instructions
To generate performance report of Digistring on a dataset, run:  
`./doplh_chebyshev_window <output_filename> <window length> <attenuation>`  
The `output_filename` will be overwritten if it already exists. Attenuation is in dB.  
In SciPy version 1.7.3, attenuation has to be >= 45 dB.

Note that SciPy offers a symmetrical and asymmetrical variant of the function. The SciPy's documentation states the asymmetric function should be used for spectral analysis. To use the symmetric version, change `sym=False` to `sym=True` in the `signal.windows.chebwin()` function call (`src/main.py`).
