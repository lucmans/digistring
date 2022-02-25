#ifndef DIGISTRING_GRAPHICS_FUNC_H
#define DIGISTRING_GRAPHICS_FUNC_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>


// Maximizes and centers src in dst without changing src's aspect ratio
constexpr void fit_center(SDL_Rect &src, const SDL_Rect &dst, const int padding/* = 0*/);

SDL_Texture *create_txt_texture(SDL_Renderer *const renderer, const std::string &text, TTF_Font *const font, const SDL_Color &color);


#endif  // DIGISTRING_GRAPHICS_FUNC_H
