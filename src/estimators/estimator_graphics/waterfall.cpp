#include "waterfall.h"

#include "spectrum.h"
#include "estimators/estimator.h"

#include "error.h"
#include "config/graphics.h"

#include <SDL2/SDL.h>

#include <list>


Waterfall::Waterfall() {
    last_max_display_frequency = -1.0;
}

Waterfall::~Waterfall() {
    for(auto &line : lines)
        SDL_DestroyTexture(line);
}


inline uint32_t calc_color(const double data, const double max_value) {
    uint32_t rgba = 0x000000ff;  // a
    const double t = (data / max_value) * 0.8;

    rgba |= (uint8_t)(9 * (1 - t) * t * t * t * 255) << 24;  // r
    rgba |= (uint8_t)(15 * (1 - t) * (1 - t) * t * t * 255) << 16;  // g
    rgba |= (uint8_t)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255) << 8;  // b

    return rgba;
}

inline unsigned int get_spectrum_size(const SpectrumData &spectrum) {
    const unsigned int size = spectrum.size();
    if(size > MAX_PIXELS_WATERFALL_LINE) {
        warning("Number of points in the spectrum for the waterfall plot is greater than MAX_PIXELS_WATERFALL_LINE (" + STR(MAX_PIXELS_WATERFALL_LINE) + ")\nLimiting to " + STR(MAX_PIXELS_WATERFALL_LINE));
        return MAX_PIXELS_WATERFALL_LINE;
    }

    return size;
}

void Waterfall::make_line(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const {
    const SpectrumData spectrum_data = spectrum.get_data();
    static const unsigned int spectrum_size = get_spectrum_size(spectrum_data);

    // Create line texture
    // SDL_Texture *new_line = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, spectrum_size, 1);
    SDL_Texture *new_line = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, spectrum_size, 1);
    if(new_line == NULL) {
        error("Failed to create line texture for waterfall plot\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // Calculate the color of each pixel
    uint32_t *pixels;
    int pitch;
    SDL_LockTexture(new_line, NULL, (void**)&pixels, &pitch);
    for(unsigned int i = 0; i < spectrum_size; i++)
        pixels[i] = calc_color(spectrum_data[i].amp, graphics_data.max_recorded_value);
    SDL_UnlockTexture(new_line);

    /*
    // Static texture (also set flag in SDL_CreateTexture above)
    uint32_t pixels[MAX_FRAME_SIZE];
    for(int i = 0; i < spectrum_size; i++)
        pixels[i] = calc_color(spectrum_data[i].amp, graphics_data.max_recorded_value);
    SDL_UpdateTexture(waterfall_line_buffer, NULL, pixels, (spectrum_size * sizeof(uint32_t));
    */

    // Add new line
    lines.push_front(new_line);

    // Keep size bound
    while((int)lines.size() > dst.h || (int)lines.size() > MAX_HISTORY_DATAPOINTS) {
        SDL_DestroyTexture(lines.back());
        lines.pop_back();
    }
}


void Waterfall::render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const {
    const SpectrumData spectrum_data = spectrum.get_data();

    // Calculate the number of pixels from the buffer to show
    // Ignore float-equal warning, as we want to run this code on any change of graphics_data.max_display_frequency
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"
    if(last_max_display_frequency != graphics_data.max_display_frequency) {
        const unsigned int spectrum_size = spectrum_data.size();
        unsigned int i;
        for(i = 0; i < spectrum_size; i++)
            if(spectrum_data[i].freq > graphics_data.max_display_frequency)
                break;

        n_pixels_per_line = i;
        last_max_display_frequency = graphics_data.max_display_frequency;
    }
    #pragma GCC diagnostic pop

    // Render the lines
    std::list<SDL_Texture *>::iterator it = lines.begin();
    const int n_lines = lines.size();
    for(int i = 0; i < n_lines && i < dst.h; i++) {
        SDL_Rect src_rect = {0, 0, n_pixels_per_line, 1};

        SDL_Rect dst_rect;
        if constexpr(WATERFALL_FLOW_DOWN)
            dst_rect = {dst.x, i + dst.y, dst.w, 1};
        else
            dst_rect = {dst.x, (dst.h - i) + dst.y, dst.w, 1};

        SDL_RenderCopy(renderer, *it, &src_rect, &dst_rect);
        it++;
    }
}
