#ifndef DIGISTRING_INIT_SDL_AUDIO_H
#define DIGISTRING_INIT_SDL_AUDIO_H


#include <SDL2/SDL.h>


void print_audio_driver();

void print_playback_devices();
void print_recording_devices();

void init_playback_device(SDL_AudioDeviceID &out_dev);
void init_recording_device(SDL_AudioDeviceID &in_dev);


#endif  // DIGISTRING_INIT_SDL_AUDIO_H
