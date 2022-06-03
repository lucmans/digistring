#include "graphics.h"

#include "config.h"
#include "error.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>
#include <chrono>


Graphics::Graphics(const double driver_latency) {
    window = SDL_CreateWindow("Playback latency", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, 0);
    if(window == NULL) {
        error("Failed to create window\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL) {
        error("Failed to create renderer for window\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }

    // Initial draw
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    const char *const font_path = "../../rsc/font/DejaVuSans.ttf";
    font = TTF_OpenFont(font_path, FONT_SIZE);
    if(font == NULL) {
        error("Failed to load font '" + STR(font_path) + "font/DejaVuSans.ttf'\nTTF error: " + TTF_GetError());
        exit(EXIT_FAILURE);
    }

    const std::string full_driver_latency_str = std::to_string(driver_latency);
    const std::string driver_latency_str = full_driver_latency_str.substr(0, full_driver_latency_str.find(".") + 1 + RENDER_DOUBLE_PRECISION);
    driver_latency_txt = create_txt_texture("Driver latency: " + driver_latency_str + " ms  (one trip)", TXT_COLOR);

    current_latency_txt = create_txt_texture("Current latency: ", TXT_COLOR);
    current_latency_ms_txt = create_txt_texture(" ms", TXT_COLOR);

    warning1_txt = create_txt_texture("These latencies only account for software latency!", TXT_COLOR);
    warning2_txt = create_txt_texture("See the readme for more information", TXT_COLOR);
}

Graphics::~Graphics() {
    SDL_DestroyTexture(warning1_txt);
    SDL_DestroyTexture(warning2_txt);

    SDL_DestroyTexture(current_latency_txt);
    SDL_DestroyTexture(current_latency_ms_txt);

    SDL_DestroyTexture(driver_latency_txt);

    TTF_CloseFont(font);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}


SDL_Texture *Graphics::create_txt_texture(const std::string &text, const SDL_Color &color) {
    // SDL_Surface *text_surface = TTF_RenderText_Solid(font, text.c_str(), color);
    SDL_Surface *text_surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if(text_surface == NULL) {
        error("Failed to render text to surface\nTTF error: " + STR(TTF_GetError()));
        exit(EXIT_FAILURE);
    }

    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if(texture == NULL) {
        error("Failed to create text from surface\nSDL error: " + STR(SDL_GetError()));
        exit(EXIT_FAILURE);
    }
    SDL_FreeSurface(text_surface);

    return texture;
}


void Graphics::render_frame(const double current_latency) {
    static std::chrono::steady_clock::time_point prev_frame = std::chrono::steady_clock::now();
    static std::chrono::duration<double, std::milli> frame_time;

    // Limit FPS
    frame_time = std::chrono::steady_clock::now() - prev_frame;
    if(frame_time.count() < 1000.0 / MAX_FPS)
        return;
    prev_frame = std::chrono::steady_clock::now();

    // Render frame
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
    SDL_RenderClear(renderer);

    int h_offset = 0;
    render_driver_latency(h_offset);
    render_current_latency(current_latency, h_offset);
    render_warning(h_offset);

    SDL_RenderPresent(renderer);
}


void Graphics::render_driver_latency(int &h_offset) {
    int w, h;
    SDL_QueryTexture(driver_latency_txt, NULL, NULL, &w, &h);
    SDL_Rect dst = {RENDER_X_OFFSET, RENDER_Y_OFFSET + h_offset, w, h};
    SDL_RenderCopy(renderer, driver_latency_txt, NULL, &dst);

    h_offset += h;
}

void Graphics::render_current_latency(const double current_latency, int &h_offset) {
    int latency_w_offset = 0;
    int w, h;
    SDL_Rect dst;

    SDL_QueryTexture(current_latency_txt, NULL, NULL, &w, &h);
    dst = {RENDER_X_OFFSET + latency_w_offset, RENDER_Y_OFFSET + h_offset, w, h};
    SDL_RenderCopy(renderer, current_latency_txt, NULL, &dst);
    latency_w_offset += w;

    const std::string full_current_latency_str = std::to_string(current_latency);
    const std::string current_latency_str = full_current_latency_str.substr(0, full_current_latency_str.find(".") + 1 + RENDER_DOUBLE_PRECISION);
    SDL_Texture *current_latency_value = create_txt_texture(current_latency_str, TXT_COLOR);
    SDL_QueryTexture(current_latency_value, NULL, NULL, &w, &h);
    dst = {RENDER_X_OFFSET + latency_w_offset, RENDER_Y_OFFSET + h_offset, w, h};
    SDL_RenderCopy(renderer, current_latency_value, NULL, &dst);
    SDL_DestroyTexture(current_latency_value);
    latency_w_offset += w;

    SDL_QueryTexture(current_latency_ms_txt, NULL, NULL, &w, &h);
    dst = {RENDER_X_OFFSET + latency_w_offset, RENDER_Y_OFFSET + h_offset, w, h};
    SDL_RenderCopy(renderer, current_latency_ms_txt, NULL, &dst);
    latency_w_offset += w;

    h_offset += h;
}

void Graphics::render_warning(int &h_offset) {
    int w, h;
    SDL_Rect dst;

    SDL_QueryTexture(warning1_txt, NULL, NULL, &w, &h);
    h_offset += h;  // Padding between warning and rest of output
    dst = {RENDER_X_OFFSET, RENDER_Y_OFFSET + h_offset, w, h};
    SDL_RenderCopy(renderer, warning1_txt, NULL, &dst);
    h_offset += h;

    SDL_QueryTexture(warning2_txt, NULL, NULL, &w, &h);
    dst = {RENDER_X_OFFSET, RENDER_Y_OFFSET + h_offset, w, h};
    SDL_RenderCopy(renderer, warning2_txt, NULL, &dst);
    h_offset += h;
}
