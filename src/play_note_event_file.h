/* TODO: Rework this entire system! */


#ifndef DIGISTRING_PLAY_NOTE_EVENT_FILE_H
#define DIGISTRING_PLAY_NOTE_EVENT_FILE_H


#include <SDL2/SDL.h>

#include <string>


void play_note_event_file(const std::string &dm_file, const SDL_AudioDeviceID &out_dev);


#endif  // DIGISTRING_PLAY_NOTE_EVENT_FILE_H
