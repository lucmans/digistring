
#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "config.h"

#include <SDL2/SDL.h>

#include <forward_list>
#include <list>


enum class PlotType {
    spectrogram, interpolated_spectrogram, waterfall
};

// List of plot types to switch between
// Graphics starts with first plot type in this list
const std::forward_list<PlotType> display_plot_type = {PlotType::spectrogram, PlotType::interpolated_spectrogram};


struct DataPoint {
    double norms[(FRAME_SIZE / 2) + 1];
    SDL_Texture *waterfall_line_buffer = NULL;

    // Destroyed in destructor of Graphics class, as destroying the renderer invalidates the texture pointer
    // ~DataPoint() {
    //     SDL_DestroyTexture(waterfall_line_buffer);
    // };
};


class Graphics {
    public:
        Graphics();
        ~Graphics();

        void set_max_recorded_value(const double new_max);
        void set_max_recorded_value_if_larger(const double new_max);
        double get_max_recorded_value_if_larger() const;

        void add_max_display_frequency(const double d_f);
        void set_max_display_frequency(const double f);
        // double get_max_display_frequency();

        void next_plot_type();

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

        double max_display_frequency;  // Maximum frequency to display; should only be set by *max_display_frequency() functions
        int n_bins;  // Maximum bins to show; should only be set by *max_display_frequency() functions
        // SDL_Texture *max_display_frequency_text;  // Rendered static text
        // SDL_Texture *max_display_frequency_number;  // Rendered dynamic text
        SDL_Texture *max_display_frequency_text;  // Rendered text

        // Used for non-interpolated spectrogram rendering
        SDL_Texture *spectrogram_buffer;


        // Render functions render to framebuffer
        void render_black_screen();

        void render_spectrogram();
        void render_interpolated_spectrogram();
        void render_waterfall();

        void render_max_displayed_frequency();
};


#endif  // GRAPHICS_H
