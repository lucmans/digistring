
#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H


#include "spectrum.h"
#include "estimators/estimator.h"

#include <vector>


class Spectrogram {
    public:
        Spectrogram();
        ~Spectrogram();

        void render_spectrogram(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const SpectrumData &spectrum_data) const;
        void render_envelope(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const SpectrumData &envelope) const;
        void render_peaks(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const std::vector<double> &peaks) const;

        // Draw line for note location in graphics
        void render_note_lines(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data) const;

        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const;
        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum, const Spectrum &envelope, const std::vector<double> &peaks) const;


    private:
        //
};


#endif  // SPECTROGRAM_H
