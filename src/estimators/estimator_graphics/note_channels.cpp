#include "note_channels.h"

#include "estimators/estimator.h"

#include <SDL2/SDL.h>

#include <algorithm>  // std::max(), std::clamp()


constexpr int CHANNEL_MAX_WIDTH = 50;  // Pixels
constexpr int CHANNEL_MAX_HEIGHT = 500;  // Pixels


NoteChannels::NoteChannels() {
    last_max_recorded_value = 0.0;
    max_power = -1.0;
}

NoteChannels::~NoteChannels() {

}


void NoteChannels::render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const NoteChannelData &note_channel_data) const {
    const int n_channels = note_channel_data.size();
    const int channel_div_width = dst.w / n_channels;

    // graphics_data.max_recorded_value should only >= every call, so if <, max_recorded_value has reset and max_power should too
    if(last_max_recorded_value > graphics_data.max_recorded_value)
        max_power = -1.0;
    last_max_recorded_value = graphics_data.max_recorded_value;

    // Check for higher max_power
    for(int i = 0; i < n_channels; i++)
        if(note_channel_data[i].power > max_power)
            max_power = note_channel_data[i].power;

    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    for(int i = 0; i < n_channels; i++) {
        int w = std::clamp(channel_div_width - 2, 1, CHANNEL_MAX_WIDTH);  // "- 2" for padding
        int h = std::max(int((note_channel_data[i].power / max_power) * dst.h), 1);
        SDL_Rect channel_bar = {(i * channel_div_width) + (int)((channel_div_width - CHANNEL_MAX_WIDTH) / 2.0) + dst.x, ((dst.h - h) - 1) + dst.y, w, h};
        SDL_RenderFillRect(renderer, &channel_bar);

    }
}
