#include "spectrogram.h"

#include "spectrum.h"
#include "estimators/estimator.h"

#include "config/transcription.h"  // LOWEST_NOTE
#include "config/graphics.h"

#include <SDL2/SDL.h>

#include <vector>
#include <cmath>


Spectrogram::Spectrogram() {

}

Spectrogram::~Spectrogram() {

}


void Spectrogram::render_spectrogram(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const SpectrumData &spectrum_data) const {
    int prev_x = 0;
    int prev_y = dst.h - 1;
    unsigned int i;
    for(i = 0; i < spectrum_data.size(); i++) {
        if(spectrum_data[i].freq > graphics_data.max_display_frequency)
            break;

        const int x = (spectrum_data[i].freq / graphics_data.max_display_frequency) * dst.w;
        const int y = dst.h - ((spectrum_data[i].amp / graphics_data.max_recorded_value) * dst.h);

        SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);

        if constexpr(DRAW_MEASURED_POINTS) {
            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0xff, 0xff);
            SDL_RenderDrawPoint(renderer, prev_x, prev_y);
        }

        prev_x = x;
        prev_y = y;
    }

    // Plot one point behind screen so graph exits screen correctly
    if(i < spectrum_data.size()) {
        const int x = (spectrum_data[i].freq / graphics_data.max_display_frequency) * dst.w;
        const int y = dst.h - ((spectrum_data[i].amp / graphics_data.max_recorded_value) * dst.h);
        SDL_SetRenderDrawColor(renderer, 0x00, 0xff, 0x00, 0xff);
        SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);

        if constexpr(DRAW_MEASURED_POINTS) {
            SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0xff, 0xff);
            SDL_RenderDrawPoint(renderer, prev_x, prev_y);
        }
    }
}


void Spectrogram::render_envelope(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const SpectrumData &envelope) const {
    if(envelope.size() == 0)
        return;

    SDL_SetRenderDrawColor(renderer, 0xff, 0x00, 0x00, 0xff);

    int prev_x = (envelope[0].freq / graphics_data.max_display_frequency) * dst.w;
    int prev_y = dst.h - ((envelope[0].amp / graphics_data.max_recorded_value) * dst.h);
    unsigned int i;
    for(i = 1; i < envelope.size(); i++) {
        if(envelope[i].freq > graphics_data.max_display_frequency)
            break;

        const int x = (envelope[i].freq / graphics_data.max_display_frequency) * dst.w;
        const int y = dst.h - ((envelope[i].amp / graphics_data.max_recorded_value) * dst.h);

        SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);

        prev_x = x;
        prev_y = y;
    }

    // Plot one point behind screen so graph exits screen correctly
    if(i < envelope.size()) {
        const int x = (envelope[i].freq / graphics_data.max_display_frequency) * dst.w;
        const int y = dst.h - ((envelope[i].amp / graphics_data.max_recorded_value) * dst.h);

        SDL_RenderDrawLine(renderer, prev_x, prev_y, x, y);
    }
}


void Spectrogram::render_peaks(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const std::vector<double> &peaks) const {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xff, 0xff);
    for(double peak_f : peaks) {
        if(peak_f > graphics_data.max_display_frequency)
            break;

        const int x = (peak_f / graphics_data.max_display_frequency) * dst.w;
        SDL_RenderDrawLine(renderer, x, 0, x, dst.h);
    }
}


void Spectrogram::render_note_lines(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data) const {
    for(double f = LOWEST_NOTE.freq; f < graphics_data.max_display_frequency; f *= exp2(1.0 / 12.0)) {
        SDL_SetRenderDrawColor(renderer, 0x30, 0x70, 0x35, 0xff);
        const int x = (f / graphics_data.max_display_frequency) * dst.w;
        SDL_RenderDrawLine(renderer, x, 0, x, dst.h);
    }
}


void Spectrogram::render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum) const {
    const SpectrumData spectrum_data = spectrum.get_data();

    if constexpr(DISPLAY_NOTE_LINES)
        render_note_lines(renderer, dst, graphics_data);

    render_spectrogram(renderer, dst, graphics_data, spectrum_data);
}

void Spectrogram::render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data, const Spectrum &spectrum, const Spectrum &envelope, const std::vector<double> &peaks) const {
    const SpectrumData spectrum_data = spectrum.get_data();
    const SpectrumData envelope_data = envelope.get_data();

    if constexpr(DISPLAY_NOTE_LINES)
        render_note_lines(renderer, dst, graphics_data);

    if constexpr(RENDER_PEAKS)
        render_peaks(renderer, dst, graphics_data, peaks);

    if constexpr(RENDER_ENVELOPE)
        render_envelope(renderer, dst, graphics_data, envelope_data);

    render_spectrogram(renderer, dst, graphics_data, spectrum_data);
}
