#ifndef GRAPHICS_H
#define GRAPHICS_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <string>


class Graphics {
    public:
        Graphics(const double driver_latency);
        ~Graphics();

        SDL_Texture *create_txt_texture(const std::string &text, const SDL_Color &color);

        void render_frame(const double current_latency);


    private:
        SDL_Window *window;
        SDL_Renderer *renderer;

        TTF_Font *font;
        SDL_Texture *driver_latency_txt;
        SDL_Texture *current_latency_txt;
        SDL_Texture *current_latency_ms_txt;
        SDL_Texture *warning1_txt;
        SDL_Texture *warning2_txt;


        void render_driver_latency(int &h_offset);
        void render_current_latency(const double current_latency, int &h_offset);
        void render_warning(int &h_offset);
};


#endif  // GRAPHICS_H
