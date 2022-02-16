
#include "graphics.h"

#include "graphics_func.h"
#include "performance.h"
#include "error.h"

#include "note.h"
#include "spectrum.h"

#include "config/cli_args.h"
#include "config/graphics.h"
#include "config/audio.h"
#include "config/transcription.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>  // std::find(), std::max()
#include <cmath>  // std::find(), std::max()


Graphics::Graphics() {
    // if(SDL_SetHintWithPriority("SDL_HINT_RENDER_SCALE_QUALITY", "2", SDL_HINT_OVERRIDE) == SDL_FALSE) {
    //     debug("Scaling hint = 1");
    //     if(SDL_SetHintWithPriority("SDL_HINT_RENDER_SCALE_QUALITY", "1", SDL_HINT_OVERRIDE) == SDL_FALSE)
    //         warning("Failed to set scaling hint; using ugly pixelated nearest neighbor scaling");
    // }

    if(cli_args.res_w == -1) {
        res_w = DEFAULT_RES[0];
        res_h = DEFAULT_RES[1];
    }
    else {
        res_w = cli_args.res_w;
        res_h = cli_args.res_h;
    }

    uint32_t window_flags;
    if(cli_args.fullscreen)
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

    plot_type = *(display_plot_type.begin());
    max_recorded_value = 1.0;
    max_display_frequency = DEFAULT_MAX_DISPLAY_FREQUENCY;
    n_waterfall_pixels = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));

    info_font = TTF_OpenFont((cli_args.rsc_dir + "font/DejaVuSans.ttf").c_str(), 20);
    if(info_font == NULL) {
        error("Failed to load font '" + cli_args.rsc_dir + "font/DejaVuSans.ttf'\nTTF error: " + TTF_GetError());
        exit(EXIT_FAILURE);
    }

    note_text = create_txt_texture(renderer, "Note: ", info_font, {0xff, 0xff, 0xff, 0xff});
    note_freq_text = create_txt_texture(renderer, "Freq: ", info_font, {0xff, 0xff, 0xff, 0xff});
    note_error_text = create_txt_texture(renderer, "Err: ", info_font, {0xff, 0xff, 0xff, 0xff});
    note_amp_text = create_txt_texture(renderer, "Amp: ", info_font, {0xff, 0xff, 0xff, 0xff});

    max_display_frequency_text = create_txt_texture(renderer, "Max displayed frequency: ", info_font, {0xff, 0xff, 0xff, 0xff});
    max_display_frequency_number = create_txt_texture(renderer, std::to_string((int)max_display_frequency), info_font, {0xff, 0xff, 0xff, 0xff});

    n_samples_text = create_txt_texture(renderer, "Queued input samples: ", info_font, {0xff, 0xff, 0xff, 0xff});
    queued_samples = -1;

    mouse_x = -1;
    clicked_freq_text = create_txt_texture(renderer, "Clicked frequency: ", info_font, {0xff, 0xff, 0xff, 0xff});

    TTF_Font *freeze_font = TTF_OpenFont((cli_args.rsc_dir + "font/DejaVuSans.ttf").c_str(), 75);
    if(freeze_font == NULL) {
        error("Failed to load font '" + cli_args.rsc_dir + "font/DejaVuSans.ttf'\nTTF error: " + TTF_GetError());
        exit(EXIT_FAILURE);
    }
    freeze = false;
    freeze_txt_buffer = create_txt_texture(renderer, "Frozen", freeze_font, {0x00, 0xff, 0xff, 0xff});
    TTF_CloseFont(freeze_font);
}

Graphics::~Graphics() {
    for(auto &dp : data_points)
        if(dp.waterfall_line_buffer != NULL)
            SDL_DestroyTexture(dp.waterfall_line_buffer);

    SDL_DestroyTexture(freeze_txt_buffer);

    SDL_DestroyTexture(clicked_freq_text);

    SDL_DestroyTexture(n_samples_text);

    SDL_DestroyTexture(note_text);
    SDL_DestroyTexture(note_freq_text);
    SDL_DestroyTexture(note_error_text);
    SDL_DestroyTexture(note_amp_text);

    SDL_DestroyTexture(max_display_frequency_text);
    SDL_DestroyTexture(max_display_frequency_number);
    TTF_CloseFont(info_font);

    SDL_DestroyTexture(frame_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


void Graphics::set_max_recorded_value(const double new_max) {
    if(new_max > -0.00001 && new_max < 0.00001) {
        warning("Can't set max_recorded_value to 0.0, keeping old value '" + STR(max_recorded_value) + '\'');
        return;
    }

    max_recorded_value = new_max;
}

void Graphics::set_max_recorded_value_if_larger(const double new_max) {
    if(new_max > max_recorded_value)
        max_recorded_value = new_max;
}

double Graphics::get_max_recorded_value() const {
    return max_recorded_value;
}


void Graphics::add_max_display_frequency(const double d_f) {
    if(max_display_frequency + d_f > MAX_FOURIER_FREQUENCY) {
        warning("Can't set maximum displayed frequency greater than " + STR(MAX_FOURIER_FREQUENCY) + " Hz; setting it to maximum");
        max_display_frequency = MAX_FOURIER_FREQUENCY;
        n_waterfall_pixels = (FRAME_SIZE / 2) + 1;
    }
    else if(max_display_frequency + d_f < MIN_FOURIER_FREQUENCY) {
        warning("Can't set maximum displayed frequency lower than " + STR(MIN_FOURIER_FREQUENCY) + "; setting it to minimum");
        max_display_frequency = MIN_FOURIER_FREQUENCY;
        n_waterfall_pixels = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    }
    else {
        max_display_frequency += d_f;
        n_waterfall_pixels = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    }

    SDL_DestroyTexture(max_display_frequency_number);
    max_display_frequency_number = create_txt_texture(renderer, std::to_string((int)max_display_frequency), info_font, {0xff, 0xff, 0xff, 0xff});

    // info("Set maximum frequency to " + STR(max_display_frequency) + " Hz");
}

void Graphics::set_max_display_frequency(const double f) {
    if(f > MAX_FOURIER_FREQUENCY) {
        warning("Can't set maximum displayed frequency greater than " + STR(MAX_FOURIER_FREQUENCY) + " Hz; setting it to maximum");
        max_display_frequency = MAX_FOURIER_FREQUENCY;
        n_waterfall_pixels = (FRAME_SIZE / 2) + 1;
    }
    else if(f < MIN_FOURIER_FREQUENCY) {
        warning("Can't set maximum displayed frequency lower than " + STR(MIN_FOURIER_FREQUENCY) + "; setting it to minimum");
        max_display_frequency = MIN_FOURIER_FREQUENCY;
        n_waterfall_pixels = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    }
    else {
        max_display_frequency = f;
        n_waterfall_pixels = ceil(max_display_frequency / ((double)SAMPLE_RATE / (double)FRAME_SIZE));
    }

    SDL_DestroyTexture(max_display_frequency_number);
    max_display_frequency_number = create_txt_texture(renderer, std::to_string((int)max_display_frequency), info_font, {0xff, 0xff, 0xff, 0xff});

    // info("Set maximum frequency to " + STR(max_display_frequency) + " Hz");
}

// double Graphics::get_max_display_frequency() {
//     return max_display_frequency;
// }


void Graphics::set_queued_samples(const int n_samples) {
    queued_samples = n_samples;
}

void Graphics::set_clicked(const int x, const int y) {
    mouse_x = x;
    mouse_y = y;
}


void Graphics::toggle_freeze_graph() {
    freeze = !freeze;
    freeze_data = (*(data_points.begin())).spectrum_data;
}


void Graphics::next_plot_type() {
    // Get to current plot type in the list of plot types to display
    std::forward_list<PlotType>::const_iterator current_type = std::find(display_plot_type.begin(), display_plot_type.end(), plot_type);
    if(current_type == display_plot_type.end()) {
        warning("Current plot type not in list of plot types to display. Graphics was likely initialized with a plot type not in the display_plot_type list.");
        plot_type = *(display_plot_type.begin());
        return;
    }

    // Get next in list
    std::forward_list<PlotType>::const_iterator next_type = ++current_type;
    if(next_type == display_plot_type.end())  // Loop to start
        next_type = display_plot_type.begin();

    // Actually set the new plot type
    plot_type = *next_type;
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

void Graphics::add_data_point(const SpectrumData *const data) {
    data_points.push_front(DataPoint());
    DataPoint &dc = *(data_points.begin());

    // Make a copy of spectrum, as it is data local to the estimator which will change
    // This assignment calls the copy constructor of std::vector
    dc.spectrum_data = *data;

    // if(env_data != nullptr)
    //     dc.envelope_data = *env_data;
    // else
    //     dc.envelope_data = EnvelopeData();

    // TODO: Support for nonlinearly spaced frequencies
    // // Make texture for waterfall plot
    // dc.waterfall_line_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STATIC, (FRAME_SIZE / 2) + 1, 1);
    // // dc.waterfall_line_buffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, (FRAME_SIZE / 2) + 1, 1);
    // if(dc.waterfall_line_buffer == NULL) {
    //     error("Failed to create texture for line of waterfall buffer\nSDL error: " + STR(SDL_GetError()));
    //     exit(EXIT_FAILURE);
    // }

    // // Static texture (also set flag in SDL_CreateTexture above)
    // uint32_t pixels[(FRAME_SIZE / 2) + 1];
    // for(int i = 0; i < (FRAME_SIZE / 2) + 1; i++)
    //     pixels[i] = calc_color(dc.spectrum_data[i].amp, max_recorded_value);
    // SDL_UpdateTexture(dc.waterfall_line_buffer, NULL, pixels, ((FRAME_SIZE / 2) + 1) * sizeof(uint32_t));

    // // Dynamic texture (also set flag in SDL_CreateTexture above)
    // // uint32_t *pixels;
    // // int pitch;
    // // SDL_LockTexture(dc.waterfall_line_buffer, NULL, (void **)&pixels, &pitch);
    // // for(int i = 0; i < (FRAME_SIZE / 2) + 1; i++)
    // //     pixels[i] = calc_color(dc.spectrum_data[i].amp, max_recorded_value);
    // // SDL_UnlockTexture(dc.waterfall_line_buffer);

    // Remove expired points
    if(data_points.size() > MAX_HISTORY_DATAPOINTS)
        data_points.pop_back();
}


void Graphics::render_frame(const Note *const note) {
    render_black_screen();

    switch(plot_type) {
        case PlotType::spectrogram:
            render_spectrogram();
            break;

        case PlotType::bins:
            render_bins();
            break;

        case PlotType::waterfall:
            render_waterfall();
            break;
    }

    render_current_note(note);
    render_max_displayed_frequency();
    if constexpr(DISPLAY_QUEUED_IN_SAMPLES)
        render_queued_samples();
    render_clicked_frequency();
    render_freeze();

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


void Graphics::render_bins() {
    SpectrumData spectrum_data = (freeze ? freeze_data : (*(data_points.begin())).spectrum_data);

    SDL_SetRenderTarget(renderer, frame_buffer);

    float x, y, bin_width;
    unsigned int i;
    SDL_SetRenderDrawColor(renderer, 0x1f, 0x77, 0xb4, 0xff);
    for(i = 0; spectrum_data[i].freq + (spectrum_data[i].bin_size / 2.0) < max_display_frequency && i < spectrum_data.size(); i++) {
        x = (spectrum_data[i].freq / max_display_frequency) * (float)res_w;
        y = res_h - ((spectrum_data[i].amp / max_recorded_value) * (float)res_h);
        bin_width = (spectrum_data[i].bin_size / max_display_frequency) * (float)res_w;

        SDL_FRect rect = {x - (bin_width / 2.0f), y, std::max(bin_width - 1.0f, 1.0f), (float)res_h};
        SDL_RenderFillRectF(renderer, &rect);
    }

    if(i < spectrum_data.size()) {
        x = (spectrum_data[i].freq / max_display_frequency) * (float)res_w;
        y = res_h - ((spectrum_data[i].amp / max_recorded_value) * (float)res_h);
        bin_width = (spectrum_data[i].bin_size / max_display_frequency) * (float)res_w;

        SDL_FRect rect = {x - (bin_width / 2.0f), y, std::max(bin_width - 1.0f, 1.0f), (float)res_h};
        SDL_RenderFillRectF(renderer, &rect);
    }
}


void Graphics::render_spectrogram() {
    SpectrumData spectrum_data = (freeze ? freeze_data : (*(data_points.begin())).spectrum_data);

    SDL_SetRenderTarget(renderer, frame_buffer);

    // Draw line for note location in graphics
    if constexpr(DISPLAY_NOTE_LINES) {
        for(double f = LOWEST_NOTE.freq; f < max_display_frequency; f *= exp2(1.0 / 12.0)) {
            SDL_SetRenderDrawColor(renderer, 0x30, 0x70, 0x35, 0xff);
            int x = (f / max_display_frequency) * res_w;
            SDL_RenderDrawLine(renderer, x, 0, x, res_h);
        }
    }

    // TODO: Envelope and peaks
    // EnvelopeData env = (*(data_points.begin())).envelope_data;
    // SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);
    // for(i = 0; i < env.size(); i++) {
    //     if(env[i].freq > max_display_frequency)
    //         break;

    //     x = (env[i].freq / max_display_frequency) * res_w;
    //     y = res_h - ((env[i].amp / max_recorded_value) * res_h);

    //     // SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    //     SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);

    //     // Draw actual measured points
    //     // SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    //     // SDL_RenderDrawPoint(renderer, prev_x, prev_y);

    //     prev_x = x;
    //     prev_y = y;
    // }

    // Plot line of spectrum
    SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
    int prev_x = 0;
    int prev_y = res_h - 1;
    int x, y;
    unsigned int i;
    for(i = 1; i < spectrum_data.size(); i++) {
        if(spectrum_data[i].freq > max_display_frequency)
            break;

        x = (spectrum_data[i].freq / max_display_frequency) * res_w;
        y = res_h - ((spectrum_data[i].amp / max_recorded_value) * res_h);

        // SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);

        // Draw actual measured points
        // SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
        // SDL_RenderDrawPoint(renderer, prev_x, prev_y);

        prev_x = x;
        prev_y = y;
    }

    // Plot one point behind screen so graph exits screen correctly
    if(i < spectrum_data.size()) {
        x = (spectrum_data[i].freq / max_display_frequency) * res_w;
        y = res_h - ((spectrum_data[i].amp / max_recorded_value) * res_h);
        SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);
    }
}


void Graphics::render_waterfall() {
    SDL_SetRenderTarget(renderer, frame_buffer);

    std::list<DataPoint>::iterator it = data_points.begin();
    int n_data_points = data_points.size();
    for(int i = 0; i < n_data_points && i < res_h; i++) {
        SDL_Rect src_rect = {0, 0, n_waterfall_pixels, 1};  // TODO: Calculate the number of points that should be used
        SDL_Rect dst_rect = {0, i, res_w, 1};
        SDL_RenderCopy(renderer, (*it).waterfall_line_buffer, &src_rect, &dst_rect);
        it++;
    }
}


void Graphics::render_current_note(const Note *const note) {
    SDL_SetRenderTarget(renderer, frame_buffer);

    int w, h, w2;
    SDL_QueryTexture(note_text, NULL, NULL, &w, &h);
    SDL_Rect dst = {0, 0, w, h};
    SDL_RenderCopy(renderer, note_text, NULL, &dst);
    if(note != nullptr) {
        SDL_Texture *note_number = create_txt_texture(renderer, note_to_string(*note), info_font, {0xff, 0xff, 0xff, 0xff});
        SDL_QueryTexture(note_number, NULL, NULL, &w2, &h);
        dst = {w, 0, w2, h};
        SDL_RenderCopy(renderer, note_number, NULL, &dst);
        SDL_DestroyTexture(note_number);
    }

    SDL_QueryTexture(note_freq_text, NULL, NULL, &w, &h);
    dst = {0, h, w, h};
    SDL_RenderCopy(renderer, note_freq_text, NULL, &dst);
    if(note != nullptr) {
        SDL_Texture *note_freq_number = create_txt_texture(renderer, STR(note->freq), info_font, {0xff, 0xff, 0xff, 0xff});
        SDL_QueryTexture(note_freq_number, NULL, NULL, &w2, &h);
        dst = {w, h, w2, h};
        SDL_RenderCopy(renderer, note_freq_number, NULL, &dst);
        SDL_DestroyTexture(note_freq_number);
    }

    SDL_QueryTexture(note_error_text, NULL, NULL, &w, &h);
    dst = {0, 2 * h, w, h};
    SDL_RenderCopy(renderer, note_error_text, NULL, &dst);
    if(note != nullptr) {
        SDL_Texture *note_error_number = create_txt_texture(renderer, STR(note->error), info_font, {0xff, 0xff, 0xff, 0xff});
        SDL_QueryTexture(note_error_number, NULL, NULL, &w2, &h);
        dst = {w, 2 * h, w2, h};
        SDL_RenderCopy(renderer, note_error_number, NULL, &dst);
        SDL_DestroyTexture(note_error_number);
    }

    SDL_QueryTexture(note_amp_text, NULL, NULL, &w, &h);
    dst = {0, 3 * h, w, h};
    SDL_RenderCopy(renderer, note_amp_text, NULL, &dst);
    if(note != nullptr) {
        SDL_Texture *note_amp_number = create_txt_texture(renderer, STR(note->amp), info_font, {0xff, 0xff, 0xff, 0xff});
        SDL_QueryTexture(note_amp_number, NULL, NULL, &w2, &h);
        dst = {w, 3 * h, w2, h};
        SDL_RenderCopy(renderer, note_amp_number, NULL, &dst);
        SDL_DestroyTexture(note_amp_number);
    }
}


void Graphics::render_max_displayed_frequency() {
    SDL_SetRenderTarget(renderer, frame_buffer);

    int w, h;
    SDL_QueryTexture(max_display_frequency_text, NULL, NULL, &w, &h);
    int w2;
    SDL_QueryTexture(max_display_frequency_number, NULL, NULL, &w2, &h);

    SDL_Rect dst = {res_w - w - w2 - 1, 0, w, h};
    SDL_RenderCopy(renderer, max_display_frequency_text, NULL, &dst);
    dst = {res_w - w2 - 1, 0, w2, h};
    SDL_RenderCopy(renderer, max_display_frequency_number, NULL, &dst);
}


void Graphics::render_queued_samples() {
    if(queued_samples == -1)
        return;

    SDL_Texture *n_samples_number = create_txt_texture(renderer, STR(queued_samples), info_font, {0xff, 0xff, 0xff, 0xff});

    int w, h;
    SDL_QueryTexture(n_samples_text, NULL, NULL, &w, &h);
    int w2;
    SDL_QueryTexture(n_samples_number, NULL, NULL, &w2, &h);

    static int max_w2 = w2;
    if(w2 > max_w2)
        max_w2 = w2;

    const int h_offset = h;  // One below max_display_frequency

    SDL_Rect dst = {res_w - w - max_w2 - 1, h_offset, w, h};
    SDL_RenderCopy(renderer, n_samples_text, NULL, &dst);
    dst = {res_w - w2 - 1, h_offset, w2, h};
    SDL_RenderCopy(renderer, n_samples_number, NULL, &dst);

    SDL_DestroyTexture(n_samples_number);
}


void Graphics::render_clicked_frequency() {
    if(mouse_x == -1)
        return;

    int clicked_freq = round(((double)mouse_x / (double)res_w) * max_display_frequency);
    SDL_Texture *clicked_freq_number = create_txt_texture(renderer, STR(clicked_freq), info_font, {0xff, 0xff, 0xff, 0xff});

    int w, h;
    SDL_QueryTexture(clicked_freq_text, NULL, NULL, &w, &h);
    int w2;
    SDL_QueryTexture(clicked_freq_number, NULL, NULL, &w2, &h);

    // If queued samples are rendered, render clicked frequency below it
    int h_offset = h;
    if(queued_samples != -1)
        h_offset += h;

    SDL_Rect dst = {res_w - w - w2 - 1, h_offset, w, h};
    SDL_RenderCopy(renderer, clicked_freq_text, NULL, &dst);
    dst = {res_w - w2 - 1, h_offset, w2, h};
    SDL_RenderCopy(renderer, clicked_freq_number, NULL, &dst);

    SDL_DestroyTexture(clicked_freq_number);
}


void Graphics::render_freeze() {
    if(!freeze)
        return;

    SDL_SetRenderTarget(renderer, frame_buffer);

    int w, h;
    SDL_QueryTexture(freeze_txt_buffer, NULL, NULL, &w, &h);

    SDL_Rect dst = {res_w - w, 0, w, h};
    SDL_RenderCopy(renderer, freeze_txt_buffer, NULL, &dst);
}
