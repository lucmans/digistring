Digistring is a program that converts notes played on a guitar to a digital representation in real-time.


# Build instructions
## Requirements
Digistring only supports Linux. It uses SDL2 for the GUI and audio input/output and FFTW3 for performing the Fourier transform.

On Arch Linux:  
`sudo pacman -S sdl2 fftw3`

## Building
Run `make` in the root directory of the project to build the binary.  
This creates the binary `digistring` in the root of the project.  
In case of failures, try running `make fresh`.


# Usage instructions
Digistring can be started by running the binary.  
If the current directory is not the root of the project, set the resource directory using the `-rsc` flag.

Sending `SIGINT` or `SIGTERM` once will queue shut-down and gracefully shut down as soon as possible.  
Sending `SIGINT` or `SIGTERM` twice will immediately stop digistring.

## Command line arguments
`-f`: Run in fullscreen. Also set the fullscreen resolution using the '-r' option.  
`-over <note> [n]`: Print n (default is 5) overtones of given note.  
`-p`: Play recorded audio back.  
`-perf`: Print performance stats to stdout.  
`-r [w] [h]`: Run digistring with given resolution.  
`-rsc <path>`: Set alternative resource directory location.  
`-s [f]`: Generate sine wave as input instead of using the recording device. Optionally, specify the frequency in hertz.

All command line arguments can also be printed by running digistring with `-h`.

## GUI controls
**Keyboard controls:**  
`-`: Decrease the frequency of the generated sine wave.  
`+`: Increase the frequency of the generated sine wave.  
`p`: Change plot type.  
`q`/`esc`: Quit digistring.


# TODO
Sleep till frame might be ready to read.


# BUGS
...
