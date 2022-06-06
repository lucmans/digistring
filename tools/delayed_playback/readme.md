Delayed playback plays back audio from a recording device to a playback device with an arbitrary latency. This is useful for determining a sensible real-time constraint for real-time audio algorithms. Furthermore, it can be used to verify the validity of real-time constraints set in papers.

The playback latency can be changed using the plus and minus keys. The latency can be reset using the r key.

Note that the latencies displayed in delayed playback are only the software latencies. Specifically, the driver buffer latency and added latency by this tool. Be sure to use a decent audio interface capable of low latency audio recording and playback.  
Furthermore, if an audio underrun occurs (not audio left in playback buffer), the playback latency will increase. Be sure your system can handle the chosen driver settings. Resetting the latency also removes latency build up from audio underruns.

# Build and run instructions
Running `make` will create a binary `delayed_playback`. Run `delayed_playback --help` for information about CLI arguments.

# TODOs
- Add headless mode.
