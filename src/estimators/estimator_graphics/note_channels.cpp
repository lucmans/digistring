#include "note_channels.h"

#include "estimators/estimator.h"

// #include "config/graphics.h"
// #include "config/transcription.h"

#include <SDL2/SDL.h>

#include <algorithm>
// #include <iostream>


constexpr int CHANNEL_MAX_WIDTH = 50;  // Pixels
constexpr int CHANNEL_MAX_HEIGHT = 500;  // Pixels


NoteChannels::NoteChannels() {

}

NoteChannels::~NoteChannels() {

}


void NoteChannels::render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const NoteChannelData &note_channel_data) const {
    const int n_channels = note_channel_data.size();
    const int channel_div_width = dst.w / n_channels;

    double max_power = -1;
    for(int i = 0; i < n_channels; i++)
        if(note_channel_data[i].power > max_power)
            max_power = note_channel_data[i].power;

    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    for(int i = 0; i < n_channels; i++) {
        int w = std::clamp(channel_div_width - 2, 1, CHANNEL_MAX_WIDTH);  // "- 2" for padding
        int h = std::max(int((note_channel_data[i].power / max_power) * dst.h), 1);
        SDL_Rect channel_bar = {(i * channel_div_width) + (int)((channel_div_width - CHANNEL_MAX_WIDTH) / 2.0) + dst.x, ((dst.h - h) - 1) + dst.y, w, h};
        SDL_RenderFillRect(renderer, &channel_bar);

        // std::cout << channel_bar.x << ' ' << channel_bar.y << ' ' << channel_bar.w << ' ' << channel_bar.h << ' ' << note_channel_data[i].power << std::endl;
    }
    // std::cout << std::endl;
}
