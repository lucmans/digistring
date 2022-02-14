Digistring is a program that converts notes played on a guitar to a digital representation in real-time.  
Note that Digistring is still under heavy development and the head of master may not always be stable.  
The thesis accompanying this project can be found in the thesis branch.


# Build instructions
## Requirements
Digistring only supports Linux. It uses SDL2 for the GUI and audio input/output and FFTW3 for performing the Fourier transform.  
The build requirements of the individual tools can be found in the tool's readme.  
Using the Dolph Chebyshev window function requires the Python program in `tools/dolph_chebyshev_window` to be able to run. See the readme in de respective directory for its requirements.

On Arch Linux:  
`sudo pacman -S sdl2 sdl2_ttf fftw3`

## Building
Run `make` in the root directory of the project to build the binary.  
This creates the binary `digistring` in the root of the project.  
In case of build failures after updating, try running `make fresh`.  
Run `make help` for more information on all build targets and make scripts.


# Usage instructions
Digistring can be started by running the binary.  
If the current directory is not the root of the project, set the resource directory using the `-rsc` flag.

Sending `SIGINT` or `SIGTERM` once will queue shut-down and gracefully exit as soon as possible.  
Sending `SIGINT` or `SIGTERM` twice will immediately stop Digistring.

## Command line arguments
Argument parameters in <> are required and in [] are optional.  
`-f`: Run in fullscreen. Also set the fullscreen resolution using the '-r' option.  
`--file <file>`: Use file as input.  
`-n [note]`: Generate note (default is A4).  
`-o | --output [file]`: Write estimation results as JSON to file (default filename is output.json).  
`--over <note> [n]`: Print n (default is 5) overtones of given note.  
`-p`: Play input audio back.  
`--perf`: Print performance stats to stdout.  
`-r <w> <h>`: Run Digistring with given resolution.  
`--rsc <path>`: Set alternative resource directory location.  
`-s [f]`: Generate sine wave as input instead of using the recording device. Optionally, specify the frequency in hertz.

All command line arguments can also be printed by running Digistring with `-h`/`--help`.

## GUI controls
**Mouse controls**  
Left mouse button: Display the frequency corresponding to the cursor's location.

**Keyboard controls:**  
`-`/`+`: Decrease/increase the frequency of the generated sine wave or note. Note that frame containing samples representing the old frequency has to finish playing before new frame is played.  
`[`/`]`: Decrease/increase the maximum displayed frequency.  
`f`: Freeze current graph.  
`p`: Change plot type.  
`r`: Reset loudest recorded value.  
`t`: Clear SDL audio playback buffer (samples sent to OS are still played).  
`q`/`esc`: Quit Digistring.


# TODO
Building requirements (GCC, Make +version of these and libs) in requirements section of this readme.  
Split config.h in multiple configuration files.  
Allow max_display_frequency > MAX_FOURIER_FREQUENCY.  
Implement Dolph-Chebyshev window in C++ (instead of calling Python using SciPy to compute the window).  
Don't copy whole input buffer when OVERLAP_NONBLOCK.  
Generic data passing structure from estimator to graphics.


# BUGS
Can't build project using "make digistring" directly due to build folder dependencies.
