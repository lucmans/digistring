Digistring is program that converts notes played on a guitar to a digital representation in real-time.

# Build instructions
## Requirements
Digistring only supports Linux. It uses SDL2 for the GUI and audio input/output and FFTW3 for performing the Fourier transform.

On Arch Linux:  
`sudo pacman -S sdl2 fftw3`

## Building
Run `make` in the root directory of the project to build the binary.  
This creates the binary `digistring` in the root of the project.  
In case of failures, try running `make force`.

# Usage instructions
TODO: Talk about running the program.

Sending `SIGINT` or `SIGTERM` once will queue application shut-down and gracefully shut down as soon as possible.  
Sending `SIGINT` or `SIGTERM` twice will immediately stop the program.

## Command line arguments
`-f`: Run in fullscreen. Also set the fullscreen resolution using the '-r' option.  
`-hl`: Run program headless. This means no window will be created and no graphics are shown.  
`p`: Play recorded audio back.  
`perf`: Print performance stats to stdout.  
`-r [w] [h]`: Run with given resolution.  
`-s`: Generate sine wave as input instead of using the recording device.
