Digistring is a program that converts notes played on a guitar to a digital representation in real-time, as well as a framework to aid development and research of real-time pitch estimation methods.  
Note that Digistring is still under heavy development and the head of master may not always be stable.  
The thesis accompanying this project can be found in the thesis branch.


# Build instructions
## Requirements
Digistring only supports Linux. It uses SDL2 for the GUI and audio input/output and FFTW3 for performing the Fourier transform.  
The build requirements of the individual tools can be found in the tool's respective readme.  
Using the Dolph Chebyshev window function requires the Python program in `tools/dolph_chebyshev_window` to be able to run. See the readme in its directory for the requirements.

On Ubuntu Linux:  
`sudo apt install libsdl2-dev libsdl2-ttf-dev libfftw3-dev`

On Arch Linux:  
`sudo pacman -S sdl2 sdl2_ttf fftw3`

## Building
Run `make` in the root directory of the project to build the binary `digistring` in the root directory of the project.  
In case of build failures after updating, try running `make fresh`.  
Run `make help` for more information on all build targets and make scripts.

## Patches
On older systems, building may fail as Digistring uses the newest features from its dependencies. Not all of these new features are necessary and some features may still be used through an older interface. To accommodate older systems, we provide some patches. These are located in the `patches` directory, along with a readme with information on when and how to use the patches. It is advised to rebuild Digistring with `make force` after applying any patches.


# Usage instructions
Digistring can be started by running the binary.  
If the current directory is not the root of the project, set the resource directory using the `--rsc` flag.

Digistring comes with tab completion, which can be used in the current session by running:  
`source completions.sh`

Sending `SIGINT` or `SIGTERM` once will queue shut-down and gracefully exit as soon as possible.  
Sending `SIGINT` or `SIGTERM` a second time will immediately stop Digistring.

## Command line arguments
Argument parameters in <> are required and in [] are optional.  
`-f`: Run in fullscreen. Also set the fullscreen resolution using the '-r' option.  
`--file <file>`: Use file as input.  
`-g | --generate-completions <file>`: Generate Bash completions to file (overwriting it).  
`-n [note]`: Generate note (default is A4).  
`-o | --output [file]`: Write estimation results as JSON to file (default filename is output.json).  
`--over <note> [n]`: Print n (default is 5) overtones of given note.  
`-p`: Play input audio back.  
`--perf`: Print performance stats to stdout.  
`-r <w> <h>`: Run Digistring with given resolution.  
`--rsc <path>`: Set alternative resource directory location.  
`-s [f]`: Generate sine wave as input instead of using the recording device. Optionally, specify the frequency in hertz.  
`--synth`: Output sine wave based on note estimation from audio input.  
`--synths`: List available synthesizers.

All command line arguments can also be printed by running Digistring with `-h`/`--help`.

## GUI controls
**Mouse controls**  
Left mouse button: Display the frequency corresponding to the cursor's location.

**Keyboard controls:**  
`-`/`+`: Decrease/increase the frequency of the generated sine wave or note. Note that frame containing samples representing the old frequency has to finish playing before new frame is played.  
`[`/`]`: Decrease/increase the maximum displayed frequency.  
`p`: Change plot type.  
`r`: Reset loudest recorded value.  
`i`: Toggle showing info in top-right corner of the window.  
`t`: Clear SDL audio playback buffer (samples sent to OS are still played).  
`q`/`esc`: Quit Digistring.


# TODO
Set cache directory location based on project root instead of relative to rsc directory.  
Ability to cache FFTW3 knowledge.  
Building requirements (GCC, Make +version of these and libs) in requirements section of this readme.  
Implement Dolph-Chebyshev window in C++ (instead of calling Python using SciPy to compute the window).  
Don't copy whole input buffer when OVERLAP_NONBLOCK.  
Plot freezing? (Was implemented before; code still partially there, but there is a new graphics rendering structure).  
Estimator graphics data wipe after frame (now have to manually .clear() old data).


# BUGS
Can't build project using "make digistring" directly due to build directory dependencies.
