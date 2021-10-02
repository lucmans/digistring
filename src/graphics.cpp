
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
    max_recorded_value = 0.0;
    max_display_frequency = DEFAULT_MAX_DISPLAY_FREQUENCY;
    n_bins = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));

    // Spectrogram plot data
    spectrogram_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, res_w, res_h);
    SDL_SetTextureBlendMode(spectrogram_buffer, SDL_BLENDMODE_BLEND);

    // TODO: create font textures
    // max_display_frequency_text = NULL;
    // max_display_frequency_number = NULL;
    // max_display_frequency_text = NULL;
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

void Graphics::add_max_display_frequency(const double d_f) {
    if(max_display_frequency + d_f > MAX_FOURIER_FREQUENCY) {
        warning("Can't set maximum displayed frequency greater than " + STR(MAX_FOURIER_FREQUENCY) + " Hz; setting it to maximum");
        max_display_frequency = MAX_FOURIER_FREQUENCY;
        n_bins = (FRAME_SIZE / 2) + 1;
    }
    else if(max_display_frequency + d_f < 2.0) {
        warning("Can't set maximum displayed frequency lower than 2; showing " + STR(MAX_FOURIER_FREQUENCY) + " Hz instead");
        max_display_frequency = 0.0;
        n_bins = (FRAME_SIZE / 2) + 1;
    }
    else {
        max_display_frequency += d_f;
        n_bins = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    }

    info("Set maximum frequency to " + STR(max_display_frequency) + " Hz");
}

void Graphics::set_max_display_frequency(const double f) {
    if(f > MAX_FOURIER_FREQUENCY) {
        warning("Can't set maximum displayed frequency greater than " + STR(MAX_FOURIER_FREQUENCY) + " Hz; setting it to maximum");
        max_display_frequency = MAX_FOURIER_FREQUENCY;
        n_bins = (FRAME_SIZE / 2) + 1;
    }
    else if(f < 2.0) {
        warning("Can't set maximum displayed frequency lower than 2; showing " + STR(MAX_FOURIER_FREQUENCY) + " Hz instead");
        max_display_frequency = 0.0;
        n_bins = (FRAME_SIZE / 2) + 1;
    }
    else {
        max_display_frequency = f;
        n_bins = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    }

    info("Set maximum frequency to " + STR(max_display_frequency) + " Hz");
}

// double Graphics::get_max_display_frequency() {
//     return max_display_frequency;
// }


void Graphics::next_plot_type() {
    switch(plot_type) {
        case PlotType::spectrogram:
            plot_type = PlotType::interpolated_spectrogram;
            break;

        case PlotType::interpolated_spectrogram:
            // plot_type = PlotType::spectrogram;
            plot_type = PlotType::waterfall;
            break;

        case PlotType::waterfall:
            plot_type = PlotType::spectrogram;
            break;
    }
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


inline uint32_t calc_color(const double data, const double max_value) {
    uint32_t rgba = 0x000000ff;  // a
    const double t = (data / max_value) * 0.8;

    rgba |= (uint8_t)(9 * (1 - t) * t * t * t * 255) << 24;  // r
    rgba |= (uint8_t)(15 * (1 - t) * (1 - t) * t * t * 255) << 16;  // g
    rgba |= (uint8_t)(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255) << 8;  // b

    return rgba;
}

void Graphics::add_data_point(const double data[(FRAME_SIZE / 2) + 1]) {
    data_points.push_front(DataPoint());
    DataPoint &dp = *(data_points.begin());

    // Copy frequency domain data over
    memcpy(dp.norms, data, ((FRAME_SIZE / 2) + 1) * sizeof(double));

    // Make texture for waterfall plot
    dp.waterfall_line_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, (FRAME_SIZE / 2) + 1, 1);
    // dp.waterfall_line_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, (FRAME_SIZE / 2) + 1, 1);
    if(dp.waterfall_line_buffer == NULL) {
        error("Failed to create texture for line of waterfall buffer\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // Static texture (also set flag in SDL_CreateTexture above)
    uint32_t pixels[(FRAME_SIZE / 2) + 1];
    for(int i = 0; i < (FRAME_SIZE / 2) + 1; i++)
        pixels[i] = calc_color(data[i], max_recorded_value);
    SDL_UpdateTexture(dp.waterfall_line_buffer, NULL, pixels, ((FRAME_SIZE / 2) + 1) * sizeof(uint32_t));

    // Dynamic texture (also set flag in SDL_CreateTexture above)
    // uint32_t *pixels;
    // int pitch;
    // SDL_LockTexture(dp.waterfall_line_buffer, NULL, (void **)&pixels, &pitch);
    // for(int i = 0; i < (FRAME_SIZE / 2) + 1; i++)
    //     pixels[i] = calc_color(data[i], max_recorded_value);
    // SDL_UnlockTexture(dp.waterfall_line_buffer);

    // Remove expired points
    if(data_points.size() > MAX_HISTORY_DATAPOINTS)
        data_points.pop_back();
}


void Graphics::render_frame() {
    render_black_screen();  // Background video if loaded

    switch(plot_type) {
        case PlotType::spectrogram:
            render_spectrogram();
            break;

        case PlotType::interpolated_spectrogram:
            render_interpolated_spectrogram();
            break;

        case PlotType::waterfall:
            render_waterfall();
            break;
    }

    render_max_displayed_frequency();

    // Render framebuffer to window
    SDL_SetRenderTarget(renderer, NULL);
    SDL_RenderCopy(renderer, frame_buffer, NULL, NULL);
    SDL_RenderPresent(renderer);
}



void Graphics::render_black_screen() {
    SDL_SetRenderTarget(renderer, frame_buffer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
}


// TODO: Fix n_bins to screen scaling
// 'Pixel' per bin
void Graphics::render_spectrogram() {
    double *norms = (*(data_points.begin())).norms;

    // Clear spectrogram_buffer
    SDL_SetRenderTarget(renderer, spectrogram_buffer);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);

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
    int prev_y = res_h - ((norms[0] / max_recorded_value) * res_h);
    for(int i = 1; i < n_bins; i++) {
        int y = res_h - ((norms[i] / max_recorded_value) * res_h);
        SDL_RenderDrawLine(renderer, i - 1, prev_y, i, y);
        prev_y = y;
    }

    // Render spectrogram_buffer to frame_buffer
    SDL_SetRenderTarget(renderer, frame_buffer);
    SDL_Rect src_spectrogram_buffer = {0, 0, n_bins, res_h};
    SDL_RenderCopy(renderer, spectrogram_buffer, &src_spectrogram_buffer, NULL);
}


// Prettier
void Graphics::render_interpolated_spectrogram() {
    double *norms = (*(data_points.begin())).norms;

    SDL_SetRenderTarget(renderer, frame_buffer);

    // Render grip
    // for(int i = 0; i < n_bins; i++)

    // TODO: Envelope and peaks

    // Plot line of spectrum
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    int prev_y = res_h - ((norms[0] / max_recorded_value) * res_h);
    for(int i = 1; i < n_bins; i++) {
        int y = res_h - ((norms[i] / max_recorded_value) * res_h);
        SDL_RenderDrawLine(renderer, (i - 1) * ((double)res_w / (double)n_bins), prev_y, i * ((double)res_w / (double)n_bins), y);
        prev_y = y;
    }
}


void Graphics::render_waterfall() {
    SDL_SetRenderTarget(renderer, frame_buffer);

    std::list<DataPoint>::iterator it = data_points.begin();
    int n_data_points = data_points.size();
    for(int i = 0; i < n_data_points && i < res_h; i++) {
        SDL_Rect src_rect = {0, 0, n_bins, 1};
        SDL_Rect dst_rect = {0, i, res_w, 1};
        SDL_RenderCopy(renderer, (*it).waterfall_line_buffer, &src_rect, &dst_rect);
        it++;
    }
}


void Graphics::render_max_displayed_frequency() {
    SDL_SetRenderTarget(renderer, frame_buffer);

    // SDL_Rect 
    // SDL_RenderCopy(renderer, max_display_frequency_text, NULL, NULL);
}
