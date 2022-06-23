This tool shows the time it takes to perform a memcpy of a large buffer of samples. The default configuration uses a buffer of 16384 samples, which equates to a frame length of 85 milliseconds at a sample rate of 192000 Hz. We chose this buffer size, as it is the buffer size at which Digistring can consistently perform pitch estimation with unoptimized parameter choices. Along with the fact that 192000 is the maximum sample rate in musical applications, it creates upper limit on buffer size. Furthermore, the preceding research project also worked well at this rate.


# Build and run instructions
Running `make` will create a binary `memcpy_speed`. The binaries take no CLI-arguments.
