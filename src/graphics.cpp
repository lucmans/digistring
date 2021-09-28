
#include "graphics.h"

#include "performance.h"
#include "config.h"
#include "error.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <cstring>


Graphics::Graphics() {
    res_w = settings.w;
    res_h = settings.h;

    uint32_t window_flags;
    if(settings.fullscreen)
        window_flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    else
        window_flags = SDL_WINDOW_UTILITY/* | SDL_WINDOW_RESIZABLE*/;

    window = SDL_CreateWindow("Digistring", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, res_w, res_h, window_flags);
    if(window == NULL) {
        error("Failed to create window\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    // SDL_GetWindowSize(window, &res_w, &res_h);
    info("Start-up resolution " + STR(res_w) + " " + STR(res_h));

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL) {
        error("Failed to create renderer for window\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    frame_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res_w, res_h);
    if(frame_buffer == NULL) {
        error("Failed to create frame buffer\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    SDL_SetTextureBlendMode(frame_buffer, SDL_BLENDMODE_BLEND);

    // TODO: Render "loading" image or splash screen
    render_black_screen();  // Initializes frame_buffer's pixels black
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, frame_buffer, NULL, NULL);
    SDL_RenderPresent(renderer);

    plot_type = PlotType::spectrogram;

    // Spectrogram plot data
    spectrogram_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res_w, res_h);
    SDL_SetTextureBlendMode(spectrogram_buffer, SDL_BLENDMODE_BLEND);

    waterfall_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, res_w, res_h);
}

Graphics::~Graphics() {
    SDL_DestroyTexture(spectrogram_buffer);

    SDL_DestroyTexture(frame_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


void Graphics::set_max_recorded_value(const double new_max) {
    max_recorded_value = new_max;
}

void Graphics::set_max_recorded_value_if_larger(const double new_max) {
    if(new_max > max_recorded_value)
        max_recorded_value = new_max;
}

double Graphics::get_max_recorded_value_if_larger() const {
    return max_recorded_value;
}


bool Graphics::resize_window(const int w, const int h) {
    if(w == res_w && h == res_h)
        return false;

    warning("Resizing might not work correctly");

    // TODO: Check if smaller than minimum resolution (MIN_RES[])

    SDL_Texture *new_frame_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    if(new_frame_buffer == NULL) {
        warning("Failed to create new framebuffer for resizing; rendering at old resolution\nSDL error: " + STR(SDL_GetError()));
        return false;
    }

    // Render current framebuffer stretched to the new screen size
    SDL_SetRenderTarget(renderer, new_frame_buffer);
    SDL_RenderCopy(renderer, frame_buffer, NULL, NULL);

    // Replace the framebuffer
    SDL_DestroyTexture(frame_buffer);
    frame_buffer = new_frame_buffer;

    res_w = w;
    res_h = h;

    return true;
}


void Graphics::add_data_point(const double data[(FRAME_SIZE / 2) + 1]) {
    DataPoint dp;
    memcpy(dp.norms, data, ((FRAME_SIZE / 2) + 1) * sizeof(double));
    data_points.push_front(dp);

    if(data_points.size() > MAX_HISTORY_DATAPOINTS)
        data_points.pop_back();
}


void Graphics::render_frame() {
    render_black_screen();  // Background video if loaded

    switch(plot_type) {
        case PlotType::spectrogram:
            render_spectrogram();
            break;

        case PlotType::waterfall:
            render_waterfall();
            break;
    }

    // Render framebuffer to window
    SDL_SetTextureBlendMode(frame_buffer, SDL_BLENDMODE_NONE);
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, frame_buffer, NULL, NULL);
    SDL_SetTextureBlendMode(frame_buffer, SDL_BLENDMODE_BLEND);
    SDL_RenderPresent(renderer);
}



void Graphics::render_black_screen() {
    SDL_SetRenderTarget(renderer, frame_buffer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
}


void Graphics::render_spectrogram() {
    double *data = (*(data_points.begin())).norms;
    int n_bins;
    if(MAX_DISPLAY_FREQ > 0)
        n_bins = ceil(MAX_DISPLAY_FREQ / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    else
        n_bins = (FRAME_SIZE / 2) + 1;

    SDL_SetRenderTarget(renderer, spectrogram_buffer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(renderer, NULL);

    // // Color found peaks
    // SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
    // for(unsigned int i = 0; i < peaks.size(); i++) {
    //     if(peaks[i] >= n_bins)
    //         continue;
    //     SDL_RenderDrawLine(renderer, peaks[i], 0, peaks[i], res_h);
    // }

    // // Plot envelope
    // SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    // int prev_y = res_h - ((envelope[0] / max_recorded_value) * res_h);
    // for(int i = 1; i < n_bins; i++) {
    //     int y = res_h - ((envelope[i] / max_recorded_value) * res_h);
    //     SDL_RenderDrawLine(renderer, i - 1, prev_y, i, y);
    //     prev_y = y;
    // }

    // Plot line of spectrum
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    int prev_y = res_h - ((data[0] / max_recorded_value) * res_h);
    for(int i = 1; i < n_bins; i++) {
        int y = res_h - ((data[i] / max_recorded_value) * res_h);
        SDL_RenderDrawLine(renderer, i - 1, prev_y, i, y);
        prev_y = y;
    }

    SDL_SetRenderTarget(renderer, frame_buffer);
    SDL_Rect src_spectrogram_buffer = {0, 0, n_bins, res_h};
    SDL_RenderCopy(renderer, spectrogram_buffer, &src_spectrogram_buffer, NULL);
}


uint32_t calc_color(const double data, const double max_value) {
    uint32_t rgba = 0x000000ff;  // a
    const double t = (data / max_value) * 0.8;

    rgba |= (uint8_t)(9 * (1 - t) * t * t * t * 255) << 24;  // r
    rgba |= (uint8_t)(15 * (1 - t) * (1 - t) * t * t * 255) << 16;  // g
    rgba |= (uint8_t)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255) << 8;  // b

    return rgba;
}

void Graphics::render_waterfall() {
    int n_bins;
    if(MAX_DISPLAY_FREQ > 0)
        n_bins = ceil(MAX_DISPLAY_FREQ / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    else
        n_bins = (FRAME_SIZE / 2) + 1;

    SDL_SetRenderTarget(renderer, waterfall_buffer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(renderer, NULL);

    int n_lines = 0;
    for(std::list<DataPoint>::iterator it = data_points.begin(); it != data_points.end(); ++it) {
        for(int i = 0; i < n_bins; i++) {
            // TODO
        }

        n_lines++;
        if(n_lines >= res_h)
            break;
    }

    SDL_SetRenderTarget(renderer, frame_buffer);
    SDL_Rect src_waterfall_buffer = {0, 0, n_bins, n_lines};
    SDL_RenderCopy(renderer, waterfall_buffer, &src_waterfall_buffer, NULL);
}
