
#ifndef GRAPHICS_H
#define GRAPHICS_H


#include "config.h"

#include "note.h"
#include "spectrum.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <forward_list>  // For display_plot_type
#include <list>  // For Graphics.data_points


enum class PlotType {
    spectrogram, bins, waterfall
};

// List of plot types to switch between
// Graphics starts with first plot type in this list
const std::forward_list<PlotType> display_plot_type = {PlotType::spectrogram, /*PlotType::waterfall,*/ PlotType::bins};


struct DataCache {
    SpectrumData spectrum_data;
    SDL_Texture *waterfall_line_buffer = NULL;
};


class Graphics {
    public:
        Graphics();
        ~Graphics();

        void set_max_recorded_value(const double new_max);
        void set_max_recorded_value_if_larger(const double new_max);
        double get_max_recorded_value() const;

        void add_max_display_frequency(const double d_f);
        void set_max_display_frequency(const double f);
        // double get_max_display_frequency();

        void set_queued_samples(const int n_samples);
        void set_clicked(const int x, const int y);

        void toggle_freeze_graph();

        void next_plot_type();

        // Returns if size was changed
        bool resize_window(const int w, const int h);

        // The SpectrumData has to be sorted on ascending frequency
        void add_data_point(const SpectrumData *const data);

        // Render the frame to the framebuffer
        void render_frame(const Note *const note);


    private:
        // Window handling
        int res_w, res_h;
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *frame_buffer;

        PlotType plot_type;
        std::list<DataCache> data_points;
        double max_recorded_value;

        //
        TTF_Font *info_font;
        SDL_Texture *note_text, *note_freq_text, *note_error_text, *note_amp_text;

        SDL_Texture *n_samples_text;
        int queued_samples;

        double max_display_frequency;  // Maximum frequency to display; should only be set by *max_display_frequency() functions
        int n_waterfall_pixels;  // Number of pixels from waterfall line buffer to write to screen
        SDL_Texture *max_display_frequency_text;  // Rendered static text
        SDL_Texture *max_display_frequency_number;  // Rendered dynamic text

        int mouse_x, mouse_y;
        SDL_Texture *clicked_freq_text;

        // Graph freezing
        bool freeze;
        SpectrumData freeze_data;
        SDL_Texture *freeze_txt_buffer;


        // Render functions render to framebuffer
        void render_black_screen();

        void render_bins();
        void render_spectrogram();
        void render_waterfall();

        void render_current_note(const Note *const note);
        void render_max_displayed_frequency();
        void render_queued_samples();
        void render_clicked_frequency();
        void render_freeze();
};


#endif  // GRAPHICS_H
