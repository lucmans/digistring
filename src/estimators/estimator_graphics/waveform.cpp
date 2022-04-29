#include "waveform.h"

#include "error.h"
#include "estimators/estimator.h"

#include <SDL2/SDL.h>

#include <vector>
#include <cmath>  // std::abs()


Waveform::Waveform() {
}

Waveform::~Waveform() {
}


void Waveform::render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const std::vector<float> &wave_samples) const {
    const int n_samples = wave_samples.size();
    if(n_samples < 2) {
        warning("Not enough samples (>2) to draw a wave");
        return;
    }

    double highest = 0.0, lowest = 0.0;
    for(const auto &sample : wave_samples) {
        if(sample > highest)
            highest = sample;
        if(sample < lowest)
            lowest = sample;
    }

    double wave_range = 2.0;
    if(highest > 1.0) {
        warning("Input clipped (above 1.0)!");
        wave_range = highest * 2.0;
    }

    if(lowest < -1.0) {
        warning("Input clipped (below -1.0)!");
        if(lowest * -2.0 > wave_range)  // Check, as highest may clip further than lowest
            wave_range = lowest * -2.0;
    }

    wave_range /= graphics_data.time_domain_y_zoom;


    const float dst_y_mid = (float)dst.y + ((float)dst.h / 2.0);
    float prev_x = dst.x;
    float prev_y = (dst_y_mid + ((wave_samples[0] / wave_range) * (float)dst.h)) + (float)dst.y;

    for(int i = 1; i < n_samples; i++) {
        const float x = (((float)i / (float)n_samples) * (float)dst.w) + (float)dst.x;
        const float y = (dst_y_mid + ((wave_samples[i] / wave_range) * (float)dst.h)) + (float)dst.y;

        // Draw clipped parts red
        if(std::abs(wave_samples[i]) > 1.0)
            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
        else
            SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);

        SDL_RenderDrawLineF(renderer, prev_x, prev_y, x, y);

        prev_x = x;
        prev_y = y;
    }
}
