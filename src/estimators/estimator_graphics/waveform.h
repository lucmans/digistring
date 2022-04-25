#ifndef DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_WAVEFORM
#define DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_WAVEFORM


#include "estimators/estimator.h"

#include <SDL2/SDL.h>

#include <vector>


class Waveform {
    public:
        Waveform();
        ~Waveform();

        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const std::vector<float> &wave_samples) const;


    private:
        //
};


#endif  // DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_WAVEFORM
