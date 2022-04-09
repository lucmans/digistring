#ifndef DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_WATERFALL_H
#define DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_WATERFALL_H


#include "spectrum.h"
#include "estimators/estimator.h"

#include <SDL2/SDL.h>

#include <list>


class Waterfall {
    public:
        Waterfall();
        ~Waterfall();

        // Builds the new waterfall line given the new spectrum
        // Should be called every perform() call; render() should only be called when plotting it
        void make_line(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const;

        // This plotter expects the frequencies in the spectrum to remain the same every call
        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const;


    private:
        // Mutable, as rendering should be const (these members are only for caching)
        mutable std::list<SDL_Texture *> lines;

        mutable double last_max_display_frequency;
        mutable unsigned int n_pixels_per_line;
};


#endif  // DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_WATERFALL_H
