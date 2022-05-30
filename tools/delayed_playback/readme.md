Delayed playback plays back audio from a recording device to a playback device with an arbitrary latency. This is useful for determining a sensible real-time constraint for real-time audio algorithms. Furthermore, it can be used to verify the validity of real-time constraints set in papers.

The playback latency can be changed using the plus and minus keys.

# Build and run instructions
Running `make` will create a binary `delayed_playback`. Run `delayed_playback --help` for information about CLI arguments.
