#include "bins.h"

#include "spectrum.h"
#include "estimators/estimator.h"

#include "config/graphics.h"
#include "config/transcription.h"

#include <SDL2/SDL.h>


Bins::Bins() {

}

Bins::~Bins() {

}


void Bins::render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const {
    const SpectrumData spectrum_data = spectrum.get_data();

    if constexpr(DISPLAY_NOTE_LINES) {
        for(double f = LOWEST_NOTE.freq; f < graphics_data.max_display_frequency; f *= exp2(1.0 / 12.0)) {
            SDL_SetRenderDrawColor(renderer, 0x30, 0x70, 0x35, 0xff);
            const int x = ((f / graphics_data.max_display_frequency) * dst.w) + dst.x;
            SDL_RenderDrawLine(renderer, x, dst.y, x, dst.h + dst.y);
        }
    }

    // Render the bins
    float h, x, y, bin_width;
    unsigned int i;
    SDL_SetRenderDrawColor(renderer, 0x1f, 0x77, 0xb4, 0xff);
    for(i = 0; i < spectrum_data.size(); i++) {
        if(spectrum_data[i].freq + (spectrum_data[i].bin_size / 2.0) > graphics_data.max_display_frequency)
            break;

        // Height of bar in pixels
        h = (spectrum_data[i].amp / graphics_data.max_recorded_value) * (float)dst.h;

        x = ((spectrum_data[i].freq / graphics_data.max_display_frequency) * (float)dst.w) + (float)dst.x;
        y = (float)(dst.h + dst.y) - h;
        bin_width = (spectrum_data[i].bin_size / graphics_data.max_display_frequency) * (float)dst.w;

        SDL_FRect rect = {x - (bin_width / 2.0f), y, std::max(bin_width - 1.0f, 1.0f), h};
        SDL_RenderFillRectF(renderer, &rect);
    }

    // Render last bin overlapping screen boundary if available
    if(i < spectrum_data.size()) {
        h = (spectrum_data[i].amp / graphics_data.max_recorded_value) * (float)dst.h;

        x = ((spectrum_data[i].freq / graphics_data.max_display_frequency) * (float)dst.w) + (float)dst.x;
        y = (float)(dst.h + dst.y) - h;
        bin_width = (spectrum_data[i].bin_size / graphics_data.max_display_frequency) * (float)dst.w;

        SDL_FRect rect = {x - (bin_width / 2.0f), y, std::max(bin_width - 1.0f, 1.0f), h};
        SDL_RenderFillRectF(renderer, &rect);
    }
}
