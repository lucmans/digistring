Digistring is a program that converts notes played on a guitar to a digital representation in real-time, as well as a framework to aid development and research of real-time pitch estimation methods.  
Note that Digistring is still under heavy development and the head of master may not always be stable.  
The thesis accompanying this project can be found in the thesis branch.


# Build instructions
## Requirements
Digistring only supports Linux. It uses g++ and Make for building. It depends on SDL2 for the GUI and audio input/output and FFTW3 for performing the Fourier transform.  
The build requirements of the individual tools can be found in the tool's respective readme.  
Optionally, in order to use the Dolph Chebyshev window function, Digistring requires Python3 and SciPy.

On Ubuntu Linux:  
`sudo apt install g++ make libsdl2-dev libsdl2-ttf-dev libfftw3-dev`  
Optional dependency for Dolph Chebyshev windows:  
`sudo apt install python3 python3-scipy`  
Note that in order to compile and run Digistring on Ubuntu, some patches have to be applied. This can be done by running `make ubuntu2104` and `make ubuntu2004lts` for Ubuntu 21.04 and Ubuntu 20.04 LTS respectively before building.

On Arch Linux:  
`sudo pacman -S gcc make sdl2 sdl2_ttf fftw3`  
Optional dependency for Dolph Chebyshev windows:  
`sudo pacman -S python3 python-scipy`

## Building
Run `make` in the root directory of the project to build the binary `digistring` in the root directory of the project.  
In case of build failures after updating, try running `make fresh`.  
Run `make help` for more information on all build targets and make scripts.  
There are some compile time settings which control logging to the terminal running Digistring. Information about these can be found in the makefile where `COMPILE_CONFIG` is defined.

## Patches
On older systems, building may fail as Digistring uses the newest features from its dependencies. Not all of these new features are necessary and some features may still be used through an older interface. To accommodate older systems, we provide some patches. These are located in the `patches` directory, along with a readme with information on when and how to use the patches. It is advised to rebuild Digistring with `make force` after applying any patches.


# Usage instructions
Digistring can be started by running the binary.  
If the current directory is not the root of the project, set the resource directory using the `--rsc` flag.

Digistring comes with tab completion, which can be used in the current session by running:  
`source completions.sh`

Sending `SIGINT` or `SIGTERM` once will queue shut-down and gracefully exit as soon as possible.  
Sending `SIGINT` or `SIGTERM` a second time will immediately stop Digistring.

## Configuration
Most of Digistring's configuration is done compile time to optimize performance and minimize latency. The default configuration is optimized for real-time usage (e.g. connecting a guitar to the audio input of your computer). The configuration files can be found in `src/config/`.  
`audio.h`: Audio driver configuration, such as sample rate, samples per buffer and sample format. Also contains the setting to enable slowdown mode.  
`transcription.h`: Contains all parameters which control pitch estimation. This includes overlapping input frames configuration.  
`graphics.h`: GUI configuration. Most important is headless mode, which ensures no graphics code is compiled into Digistring. This is important, as graphics is only useful for research/debugging and adds much CPU and RAM overhead. Headless mode is useful for practical usage (real-time sound synthesis based on guitar input) and experimental usage (running performance measurements).

## Command line arguments
Argument parameters in <> are required and in [] are optional.  
`--audio_in <device name>`: Set the recording device to device name (as provided by Digistring at start-up).  
`--audio_out <device name>`: Set the playback device to device name (as provided by Digistring at start-up).  
`-f`: Run in fullscreen. Also set the fullscreen resolution using the '-r' option.  
`--file <file>`: Use file as input.  
`--gen-completions <file>`: Generate Bash completions to file (overwriting it).  
`-n [note]`: Generate note (default is A4).  
`-o | --output [file]`: Write estimation results as JSON to file (default filename is output.json).  
`--over <note> [n] [midi]`: Print n (default is 5) overtones of given note; optionally toggle midi number column by passing "midi_on" or "midi_off" (default to "midi_off").  
`-p [left/right]`: Play input audio back. When also synthesizing, pass "left" or "right" to set playback to this channel (and synthesis to the other).  
`--perf`: Print performance stats to stdout.  
`-r <w> <h>`: Run Digistring with given resolution.  
`--rsc <path>`: Set alternative resource directory location.  
`-s [f]`: Generate sine wave as input instead of using the recording device. Optionally, specify the frequency in hertz.  
`--sync`: Run Digistring "real-time"; in other words, sync graphics etc. as if audio was playing back.  
`--synth [synth_type]`: Generate sound based on note estimation (default is sine).  
`--synths`: List available synthesizers (`synth_type`s for `--synth`).

All command line arguments can also be printed by running Digistring with `-h`/`--help`.

## GUI controls
**Mouse controls**  
Left mouse button: Display the frequency corresponding to the cursor's location.

**Keyboard controls:**  
`-`/`+`: Decrease/increase the frequency of the generated sine wave or note. Note that frame containing samples representing the old frequency has to finish playing before new frame is played.  
`[`/`]`: Decrease/increase the maximum displayed frequency.  
`,`/`.`: Zoom in/out waveform plot.  
`p`: Change plot type.  
`r`: Reset loudest recorded value.  
`i`: Toggle showing info in top-right corner of the window.  
`t`: Clear SDL audio playback buffer (samples sent to OS are still played).  
`q`/`esc`: Quit Digistring.


# Tools
Digistring includes a few tools:  
`dolph_chebyshev_window`: A Python program which calculates the Dolph Chebyshev window. Is used directly by Digistring.  
`float_vs_double`: Tests if single precision (`float`) or double precision (`double`) floating point calculations are faster.  
`generate_report`: Generates a performance report based on Digistring's output compared to ground truth annotation.  
`patch_tools`: A few tools which help with checking, applying and creating patches.


# TODO
- Adjustable synth volume.  
- Program::slowdown() without std::move().  
- Chance sqrt(x^2 + y^2) to hypot(x, y).  
- Make SLOWDOWN a cli arg instead of compile time parameter.  
- Make SampleGetter::WaveGenerator a baseclass and change current WaveGenerator to SineGenerator subclass.  
- Make SampleGetter and Estimator pointer in Program const (using factories to create them in constructor initializer list).  
- Put estimation_func.{cpp,h} and helper functions in highres.cpp in estimation_func directory.  
- Pass SampleGetter to Estimator and add get_samples() to base class (which doesn't overlap).  
- Convex envelope and low passed-spectrum peak picking.  
- Correct signal power and note dB calculation.  
- Building requirements (GCC, Make +version of these and libs) in requirements section of this readme.  
- Ability to cache FFTW3 knowledge.  
- Don't copy whole input buffer when OVERLAP_NONBLOCK.  
- Plot freezing? (Was implemented before; code still partially there, but there is a new graphics rendering structure).  
- Implement Dolph-Chebyshev window in C++ (instead of calling Python using SciPy to compute the window).  
- Set cache directory location based on project root instead of relative to rsc directory.  
- Estimator graphics data wipe after frame (now have to manually .clear() old data).


# BUGS
Audio click and pops from synthesizer when using NoteEvent offset and length.
