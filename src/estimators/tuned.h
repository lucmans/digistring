#ifndef DIGISTRING_ESTIMATORS_TUNED_H
#define DIGISTRING_ESTIMATORS_TUNED_H


#include "estimator.h"

#include <fftw3.h>

#include "note.h"

#include "estimator_graphics/spectrum.h"
#include "estimator_graphics/spectrogram.h"
#include "estimator_graphics/bins.h"


class Tuned : public Estimator {
    public:
        Tuned(float *&input_buffer, int &buffer_size);
        ~Tuned() override;

        Estimators get_type() const override;

        void perform(float *const input_buffer, NoteEvents &note_events) override;


    private:
        int buffer_sizes[12];
        float *ins[12];
        fftwf_complex *outs[12];
        fftwf_plan plans[12];

        double *norms;
        float *window_funcs[12];
};


class TunedGraphics : public EstimatorGraphics {
    public:
        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data) const override {
            switch(cur_plot) {
                default:
                    cur_plot = 0;
                    __attribute__ ((fallthrough));
                case 0:
                    spectrogram.render(renderer, dst, graphics_data, spectrum);
                    break;

                case 1:
                    bins.render(renderer, dst, graphics_data, spectrum);
                    break;
            }
        };

        // These should only be called by the Tuned estimator in perform()
        Spectrum &get_spectrum() {return spectrum;};
        // Spectrum &get_envelope() {return envelope;};
        // std::vector<double> &get_peaks() {return peak_frequencies;};


    private:
        Spectrogram spectrogram;
        Bins bins;

        // These get set during a perform() call
        Spectrum spectrum;
        // Spectrum envelope;
        // std::vector<double> peak_frequencies;
};


#endif  // DIGISTRING_ESTIMATORS_TUNED_H
