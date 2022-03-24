#ifndef DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_NOTE_CHANNELS_H
#define DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_NOTE_CHANNELS_H


#include "note.h"
#include "estimators/estimator.h"

#include <SDL2/SDL.h>

#include <vector>


struct NoteChannelDataPoint {
    // Note note;
    double power;

    NoteChannelDataPoint(const double _p) : power(_p) {};
};

typedef std::vector<NoteChannelDataPoint> NoteChannelData;


class NoteChannels {
    public:
        NoteChannels();
        ~NoteChannels();

        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const NoteChannelData &note_channel_data) const;


    private:
        // Mutable, as rendering should be const (these members are only for caching)
        mutable double last_max_recorded_value;
        mutable double max_power;
};


#endif  // DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_NOTE_CHANNELS_H
