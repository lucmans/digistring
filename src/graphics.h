#ifndef DIGISTRING_GRAPHICS_H
#define DIGISTRING_GRAPHICS_H


#include "note.h"
#include "estimators/estimator.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


class Graphics {
    public:
        Graphics();
        ~Graphics();

        void set_max_recorded_value();
        void set_max_recorded_value(const double new_max);
        void set_max_recorded_value_if_larger(const double new_max);
        double get_max_recorded_value() const;

        void add_max_display_frequency(const double d_f);
        void set_max_display_frequency(const double f);
        double get_max_display_frequency() const;

        void set_queued_samples(const int n_samples);
        void set_clicked(const int x, const int y);

        void toggle_show_info();

        // Returns if size was changed
        bool resize_window(const int w, const int h);

        // Render the frame to the framebuffer and framebuffer to screen
        void render_frame(const Note *const note, const EstimatorGraphics *const estimator_graphics);


    private:
        // Window handling
        int res_w, res_h;
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *frame_buffer;

        //
        TTF_Font *info_font;
        SDL_Texture *note_text, *note_freq_text, *note_error_text, *note_amp_text;
        bool show_info;

        SDL_Texture *n_samples_text;
        int queued_samples;

        double max_display_frequency;  // Maximum frequency to display; should only be set by set_max_display_frequency() functions
        SDL_Texture *max_display_frequency_text;  // Rendered static text
        SDL_Texture *max_display_frequency_number;  // Rendered dynamic text

        double max_recorded_value;  // Should only be set by set_max_recorded_value*() functions
        SDL_Texture *max_recorded_value_text;  // Rendered static text
        SDL_Texture *max_recorded_value_number;  // Rendered dynamic text

        int mouse_x, mouse_y;
        SDL_Texture *clicked_freq_text;
        SDL_Texture *clicked_amp_text;


        // Render functions render to framebuffer
        void render_black_screen();

        void render_current_note(const Note *const note);

        void render_info();
        void render_max_displayed_frequency(int &offset);
        void render_max_recorded_value(int &offset);
        void render_queued_samples(int &offset);
        void render_clicked_location_info(int &offset);
};


#endif  // DIGISTRING_GRAPHICS_H
