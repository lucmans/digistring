This tool can show if there is a performance difference between using floats and doubles by calculating the Mandelbrot set using floats and doubles and timing how long it takes for each.  
Before starting the measurement calculations, a dry run is done to "warm up" the processor. This makes sure that performance scaling CPUs are at max performance before starting the experiment.

The Mandelbrot set is calculated on the domain Real=[-2.0, 1.0] and Imag=[-1.5, 1.5]. The number of calculated pixels and maximum iterations can be configured at the top of `main.cpp`.

High number of pixels may cause stack overflows. In this case, lower the number of pixels and increase the maximum number of iterations.


# Build and run instructions
Running `make` will create three different binaries. `float_vs_double` is compiled without optimizations, `float_vs_double_O3` only with `-O3` and `float_vs_double_fast` with many extra optimizations (such as fused multiply add and vectorization).

The binaries take no CLI-arguments.
