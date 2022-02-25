#include "graphics.h"

#include "graphics_func.h"
#include "performance.h"
#include "error.h"

#include "note.h"
#include "estimators/estimator.h"

#include "config/cli_args.h"
#include "config/graphics.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


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

    max_recorded_value = 1.0;
    max_display_frequency = DEFAULT_MAX_DISPLAY_FREQUENCY;

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

    // TTF_Font *freeze_font = TTF_OpenFont((cli_args.rsc_dir + "font/DejaVuSans.ttf").c_str(), 75);
    // if(freeze_font == NULL) {
    //     error("Failed to load font '" + cli_args.rsc_dir + "font/DejaVuSans.ttf'\nTTF error: " + TTF_GetError());
    //     exit(EXIT_FAILURE);
    // }
    // freeze = false;
    // freeze_txt_buffer = create_txt_texture(renderer, "Frozen", freeze_font, {0x00, 0xff, 0xff, 0xff});
    // TTF_CloseFont(freeze_font);
}

Graphics::~Graphics() {
    // SDL_DestroyTexture(freeze_txt_buffer);

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
    if(new_max < 1.0) {
        warning("Can't set max_recorded_value lower than 1.0; setting it to 1.0...");
        max_recorded_value = 1.0;
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
    if(max_display_frequency + d_f < MIN_MAX_DISPLAY_FREQUENCY) {
        warning("Can't set maximum displayed frequency lower than " + STR(MIN_MAX_DISPLAY_FREQUENCY) + "; setting it to minimum");
        max_display_frequency = MIN_MAX_DISPLAY_FREQUENCY;
    }
    else {
        max_display_frequency += d_f;
    }

    SDL_DestroyTexture(max_display_frequency_number);
    max_display_frequency_number = create_txt_texture(renderer, std::to_string((int)max_display_frequency), info_font, {0xff, 0xff, 0xff, 0xff});

    // info("Set maximum frequency to " + STR(max_display_frequency) + " Hz");
}

void Graphics::set_max_display_frequency(const double f) {
    if(f < MIN_MAX_DISPLAY_FREQUENCY) {
        warning("Can't set maximum displayed frequency lower than " + STR(MIN_MAX_DISPLAY_FREQUENCY) + "; setting it to minimum");
        max_display_frequency = MIN_MAX_DISPLAY_FREQUENCY;
    }
    else {
        max_display_frequency = f;
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


// void Graphics::toggle_freeze_graph() {
//     freeze = !freeze;
//     freeze_data = (*(data_points.begin())).spectrum_data;
// }


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


void Graphics::render_frame(const Note *const note, const EstimatorGraphics *const estimator_graphics) {
    render_black_screen();

    const GraphicsData gd = {.max_display_frequency = max_display_frequency,
                             .max_recorded_value = max_recorded_value};
    estimator_graphics->render(renderer, {0, 0, res_w, res_h}, gd);

    render_current_note(note);
    render_max_displayed_frequency();
    if constexpr(DISPLAY_QUEUED_IN_SAMPLES)
        render_queued_samples();
    render_clicked_frequency();
    // render_freeze();

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


// void Graphics::render_freeze() {
//     if(!freeze)
//         return;

//     SDL_SetRenderTarget(renderer, frame_buffer);

//     int w, h;
//     SDL_QueryTexture(freeze_txt_buffer, NULL, NULL, &w, &h);

//     SDL_Rect dst = {res_w - w, 0, w, h};
//     SDL_RenderCopy(renderer, freeze_txt_buffer, NULL, &dst);
// }
