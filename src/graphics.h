
#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "config.h"

#include <SDL2/SDL.h>

#include <list>


enum class PlotType {
    spectrogram, waterfall
};


struct DataPoint {
    double norms[(FRAME_SIZE / 2) + 1];
};


class Graphics {
    public:
        Graphics();
        ~Graphics();

        void set_max_recorded_value(const double new_max);
        void set_max_recorded_value_if_larger(const double new_max);
        double get_max_recorded_value_if_larger() const;

        // Returns if size was changed
        bool resize_window(const int w, const int h);

        void add_data_point(const double data[(FRAME_SIZE / 2) + 1]);

        // Render the frame to the framebuffer
        void render_frame();


    private:
        // Window handling
        int res_w, res_h;
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *frame_buffer;

        PlotType plot_type;
        std::list<DataPoint> data_points;
        double max_recorded_value;

        SDL_Texture *spectrogram_buffer;
        SDL_Texture *waterfall_buffer;


        // Render functions render to framebuffer
        void render_black_screen();

        void render_spectrogram();
        void render_waterfall();
};


#endif  // GRAPHICS_H
