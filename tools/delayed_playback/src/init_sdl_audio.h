#ifndef INIT_SDL_AUDIO_H
#define INIT_SDL_AUDIO_H


#include <SDL2/SDL.h>


void print_audio_driver();

void print_playback_devices();
void print_recording_devices();

int init_playback_device(SDL_AudioDeviceID &out_dev);
int init_recording_device(SDL_AudioDeviceID &in_dev);


#endif  // INIT_SDL_AUDIO_H
