#ifndef DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_BINS_H
#define DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_BINS_H


#include "spectrum.h"
#include "estimators/estimator.h"

#include <SDL2/SDL.h>


class Bins {
    public:
        Bins();
        ~Bins();

        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const;


    private:
        //
};


#endif  // DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_BINS_H
