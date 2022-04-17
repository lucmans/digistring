#ifndef DIGISTRING_CONFIG_SYNTH_H
#define DIGISTRING_CONFIG_SYNTH_H


// Let AMP_SCALING be x, the amplitude of the out wave is calculated using note.amp^x / max_amp^x
constexpr double AMP_SCALING = 3.5;

// TODO: Use new_samples in Program::synthesize() to prevent slowed playback
// Only synth samples for new samples (and not for overlapped samples, which causes slowed playback)
constexpr bool SLOWDOWN_ON_OVERLAP = true;


#endif  // DIGISTRING_CONFIG_SYNTH_H
