#ifndef DIGISTRING_ESTIMATORS_HIGHRES_H
#define DIGISTRING_ESTIMATORS_HIGHRES_H


#include "estimator.h"

#include "performance.h"

#include "estimator_graphics/spectrogram.h"
#include "estimator_graphics/bins.h"
#include "estimator_graphics/waterfall.h"
#include "estimator_graphics/waveform.h"

#include "config/transcription.h"
#include "config/graphics.h"

#include <fftw3.h>

#include <vector>


class HighRes : public Estimator {
    public:
        HighRes(float *&input_buffer, int &buffer_size);
        ~HighRes() override;

        Estimators get_type() const override;

        // Note that when modifying this algorithm, the same modifications have to be made to experiments/qifft.cpp
        void perform(float *const input_buffer, NoteEvents &note_events) override;


    private:
        float *in;
        fftwf_complex *out;
        fftwf_plan p;

        float window_func[FRAME_SIZE];

        double prev_power;

        Performance perf;


        void interpolate_peaks(NoteSet &noteset, const double norms[(FRAME_SIZE_PADDED / 2) + 1], const std::vector<int> &peaks);
};


class HighResGraphics : public EstimatorGraphics {
    public:
        void render(SDL_Renderer *const renderer, const SDL_Rect &dst, const GraphicsData &graphics_data) const override {
            waterfall.make_line(renderer, dst, graphics_data, spectrum);

            switch(cur_plot) {
                default:
                    cur_plot = 0;
                    __attribute__ ((fallthrough));
                case 0:
                    spectrogram.render(renderer, dst, graphics_data, spectrum, envelope, peak_frequencies);
                    break;

                case 1:
                    bins.render(renderer, dst, graphics_data, spectrum);
                    break;

                case 2:
                    waterfall.render(renderer, dst, graphics_data, spectrum);
                    break;

                case 3:
                    waveform.render(renderer, dst, graphics_data, wave_samples);
                    break;
            }
        };

        // These should only be called by the HighRes estimator in perform()
        Spectrum &get_spectrum() {return spectrum;};
        Spectrum &get_envelope() {return envelope;};
        std::vector<double> &get_peaks() {return peak_frequencies;};
        std::vector<float> &get_wave_samples() {return wave_samples;};


    private:
        Spectrogram spectrogram;
        Bins bins;
        Waterfall waterfall;
        Waveform waveform;

        // These get set during a perform() call
        Spectrum spectrum;
        Spectrum envelope;
        std::vector<double> peak_frequencies;
        std::vector<float> wave_samples;
};


#endif  // DIGISTRING_ESTIMATORS_HIGHRES_H
