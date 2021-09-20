
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

    plot_type = PlotType::spectrogram_log;

    // Spectrogram plot data
    spectrogram_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res_w, res_h);
    SDL_SetTextureBlendMode(spectrogram_buffer, SDL_BLENDMODE_BLEND);
}

Graphics::~Graphics() {
    SDL_DestroyTexture(spectrogram_buffer);

    SDL_DestroyTexture(frame_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
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
    for(int i = 0; i < (FRAME_SIZE / 2) + 1; i++)
        if(dp.norms[i] > max_recorded_value)
            max_recorded_value = dp.norms[i];

    data_points.push_front(dp);
}


void Graphics::render_frame() {
    render_black_screen();  // Background video if loaded

    switch(plot_type) {
        case PlotType::spectrogram:
            render_spectrogram();
            break;

        case PlotType::spectrogram_log:
            render_spectrogram_log();
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
    int n_data_points = (FRAME_SIZE / 2) + 1;
    double *data = (*(data_points.begin())).norms;

    SDL_SetRenderTarget(renderer, spectrogram_buffer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(renderer, NULL);

    // // Color found peaks
    // SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
    // for(unsigned int i = 0; i < peaks.size(); i++) {
    //     if(peaks[i] >= n_data_points)
    //         continue;
    //     SDL_RenderDrawLine(renderer, peaks[i], 0, peaks[i], res_h);
    // }

    // // Plot envelope
    // SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    // int prev_y = res_h - ((envelope[0] / max_recorded_value) * res_h);
    // for(int i = 1; i < n_data_points; i++) {
    //     int y = res_h - ((envelope[i] / max_recorded_value) * res_h);
    //     SDL_RenderDrawLine(renderer, i - 1, prev_y, i, y);
    //     prev_y = y;
    // }

    // Plot line of spectrum
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    int prev_y = res_h - ((data[0] / max_recorded_value) * res_h);
    for(int i = 1; i < n_data_points; i++) {
        int y = res_h - ((data[i] / max_recorded_value) * res_h);
        SDL_RenderDrawLine(renderer, i - 1, prev_y, i, y);
        prev_y = y;
    }

    SDL_SetRenderTarget(renderer, frame_buffer);
    SDL_Rect src_spectrogram_buffer = {0, 0, n_data_points, res_h};
    SDL_RenderCopy(renderer, spectrogram_buffer, &src_spectrogram_buffer, NULL);
}

void Graphics::render_spectrogram_log() {
    int n_data_points = (FRAME_SIZE / 2) + 1;
    double *data = (*(data_points.begin())).norms;

    SDL_SetRenderTarget(renderer, spectrogram_buffer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderFillRect(renderer, NULL);

    // // Color found peaks
    // SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
    // for(unsigned int i = 0; i < peaks.size(); i++) {
    //     if(peaks[i] >= n_data_points)
    //         continue;
    //     SDL_RenderDrawLine(renderer, peaks[i], 0, peaks[i], res_h);
    // }

    // // Plot envelope
    // SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    // int prev_y = res_h - ((log10(envelope[0] + 1) / log10(max_recorded_value + 1)) * res_h);
    // for(int i = 1; i < n_data_points; i++) {
    //     int y = res_h - ((log10(envelope[i] + 1) / log10(max_recorded_value + 1)) * res_h);
    //     SDL_RenderDrawLine(renderer, i - 1, prev_y, i, y);
    //     prev_y = y;
    // }

    // Plot line of spectrum
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    int prev_y = res_h - ((log10(data[0] + 1) / log10(max_recorded_value + 1)) * res_h);
    for(int i = 1; i < n_data_points; i++) {
        int y = res_h - ((log10(data[i] + 1) / log10(max_recorded_value + 1)) * res_h);
        SDL_RenderDrawLine(renderer, i - 1, prev_y, i, y);
        prev_y = y;
    }

    SDL_SetRenderTarget(renderer, frame_buffer);
    SDL_Rect src_spectrogram_buffer = {0, 0, n_data_points, res_h};
    SDL_RenderCopy(renderer, spectrogram_buffer, &src_spectrogram_buffer, NULL);
}


void Graphics::render_waterfall() {

}
