This tool can show if there is a performance difference between using floats and doubles in FFTW3 by performing many large transformations using floats and doubles and timing how long it takes for each.

As data, the input buffer is filled with a sine wave. The size of the transform and the number of repetitions can be configured in `config.h`.


# Build and run instructions
Running `make` will create two different binaries, `float` and `double` which perform the transformations with floats and doubles respectively.

The binaries take no CLI-arguments.
