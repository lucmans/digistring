
#include "graphics_func.h"

#include "config.h"
#include "error.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>


void fit_center(SDL_Rect &src, const SDL_Rect &dst, const int padding = 0) {
    const double src_ar = src.w / src.h,
                 dst_ar = dst.w / dst.h;

    // src is narrower
    if(src_ar < dst_ar) {
        src.h = dst.h - (2 * padding);
        src.w = src.h * src_ar;
        src.x = (double)(dst.w - (src.h * src_ar)) / 2.0;
        src.y = padding;
    }
    // src is wider
    else {  //if(src_ar > dst_ar) {
        src.w = dst.w - (2 * padding);
        src.h = src.w / src_ar;
        src.x = padding;
        src.y = (double)(dst.h - (src.w / src_ar)) / 2.0;
    }
}


SDL_Texture *create_txt_texture(SDL_Renderer *const renderer, const std::string &text, TTF_Font *const font, const SDL_Color &color) {
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
