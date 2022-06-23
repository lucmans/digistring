/* TODO: Rework this entire system! */


#include "play_note_event_file.h"

#include "note.h"
#include "quit.h"
#include "error.h"
#include "synth/synths.h"

#include "config/synth.h"
#include "config/cli_args.h"

#include <SDL2/SDL.h>

#include <string>
#include <fstream>
#include <vector>
#include <algorithm>  // std::sort(), std::clamp()


struct PlayNoteEvent {
    Note note;

    // In seconds
    double onset;
    double offset;

    // In [0.0, 1.0]
    double volume;

    constexpr PlayNoteEvent(const Note &_note, const double _onset, const double _offset) : note(_note), onset(_onset), offset(_offset), volume(1.0) {};
    constexpr PlayNoteEvent(const Note &_note, const double _onset, const double _offset, const double _volume) : note(_note), onset(_onset), offset(_offset), volume(_volume) {};
};
typedef std::vector<PlayNoteEvent> PlayNoteEvents;

bool play_note_event_sorter(const PlayNoteEvent &l, const PlayNoteEvent &r) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"
    if(l.onset == r.onset)
        return l.offset < r.offset;
    #pragma GCC diagnostic pop

    return l.onset < r.onset;
}


void play_note_event_file(const std::string &note_event_file, const SDL_AudioDeviceID &out_dev) {
    // Read the file and translate to PlayNoteEvent structure
    std::ifstream events_file(note_event_file);
    if(!events_file.is_open()) {
        error("Failed to open note events file '" + note_event_file + "'");
        exit(EXIT_FAILURE);
    }

    bool offsets;
    std::string header;
    std::getline(events_file, header);
    if(header == "duration")
        offsets = false;
    else if(header == "offset")
        offsets = true;
    else {
        error("Invalid header; expected either 'duration' or 'offset' on first line");
        exit(EXIT_FAILURE);
    }

    PlayNoteEvents pne;
    // int line = 0;
    while(!poll_quit()) {  // while(true), as end of file (break condition) has to be checked right after a read
        int note_number;
        double onset, offset;//, volume = -1.0;

        events_file >> note_number >> onset >> offset;// >> volume;
        // line++;
        if(!events_file)
            break;

        // TODO: Volume support
        // if(volume == -1.0) {
        //     debug("No");
        //     volume = 1.0;
        // }
        // else
        //     debug(STR(volume));

        // if(volume < 0.0 || volume > 1.0) {
        //     warning("Invalid volume on line " + STR(line) + "; clamping it between 0.0 and 1.0");
        //     volume = std::clamp(volume, 0.0, 1.0);
        // }

        if(offsets) {
            pne.push_back(PlayNoteEvent(Note(note_number), onset, offset/*, volume*/));
        }
        else {  // if offsets field contains durations instead
            const double &duration = offset;
            pne.push_back(PlayNoteEvent(Note(note_number), onset, onset + duration/*, volume*/));
        }
    }

    if(!events_file.eof()) {
        error("Failed to read entire file");
        exit(EXIT_FAILURE);
    }

    std::sort(pne.begin(), pne.end(), play_note_event_sorter);
    for(const auto &i : pne)
        std::cout << i.note.midi_number << " " << i.onset << " " << i.offset << std::endl;

    // Play the PlayNoteEvent structure
    float *synth_buffer;
    try {
        synth_buffer = new float[SYNTH_BUFFER_SIZE];
    }
    catch(const std::bad_alloc &e) {
        error("Failed to allocate synth_buffer (" + STR(e.what()) + ")");
        hint("Try using setting a lower SYNTH_BUFFER_SIZE in config/synth.h");
        exit(EXIT_FAILURE);
    }
    Synth *const synth = synth_factory(cli_args.synth_type);
    synth->set_max_amp(1.0);
    info("Using a synth buffer size of " + STR(SYNTH_BUFFER_SIZE) + " samples at a sample rate of " + STR(SAMPLE_RATE) + " Hz");

    const int n_events = pne.size();
    int min_idx = 0;
    double played_time = 0.0;
    SDL_PauseAudioDevice(out_dev, 0);
    while(!poll_quit()) {
        const double frame_time = (double)SYNTH_BUFFER_SIZE / (double)SAMPLE_RATE;
        const double frame_ended_time = played_time + frame_time;

        // Skip events completely in the past
        for(int i = min_idx; i < n_events; i++) {
            if(pne[i].offset < played_time)
                min_idx = i;
            else
                break;
        }
        // All events are played
        // TODO: Better exit condition
        if(min_idx == n_events - 1 && pne[min_idx].offset < played_time)
            break;

        NoteEvents ne;
        for(int i = min_idx; i < n_events; i++) {
            // Break if event is in the future, as all subsequent events will also be
            if(pne[i].onset >= frame_ended_time)
                break;

            // Continue if event already finished, but can't be removed by previous for loop
            if(pne[i].offset < played_time)
                continue;

            // TODO: Actual timing; but requires polyphonic synth
            // const int start = std::clamp((int)((pne[i].onset - played_time) * (double)SAMPLE_RATE), 0, SYNTH_BUFFER_SIZE);
            // const int end = std::clamp((int)((pne[i].offset - played_time) * (double)SAMPLE_RATE), 0, SYNTH_BUFFER_SIZE);
            // ne.push_back(NoteEvent(Note(pne[i].note.midi_number, pne[i].volume), end - start, start));
            // std::cout << start << " " << end << std::endl;
            // std::cout << pne[i].onset << ' ' << pne[i].offset << "    " << played_time << " " << frame_ended_time << std::endl;
            ne.push_back(NoteEvent(Note(pne[i].note.midi_number, pne[i].volume), SYNTH_BUFFER_SIZE, 0));
        }
        // std::cout << std::endl;

        synth->synthesize(ne, synth_buffer, SYNTH_BUFFER_SIZE);
        if(SDL_QueueAudio(out_dev, synth_buffer, SYNTH_BUFFER_SIZE * sizeof(float))) {
            error("Failed to queue audio for playback\nSDL error: " + STR(SDL_GetError()));
            delete[] synth_buffer;
            delete synth;
            exit(EXIT_FAILURE);
        }

        played_time += frame_time;
    }

    // Let SDL play out all buffered audio
    while(SDL_GetQueuedAudioSize(out_dev) > 0 && !poll_quit());


    delete[] synth_buffer;
    delete synth;
}
