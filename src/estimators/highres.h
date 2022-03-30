#ifndef DIGISTRING_ESTIMATORS_HIGHRES_H
#define DIGISTRING_ESTIMATORS_HIGHRES_H


#include "estimator.h"

#include "estimator_graphics/spectrogram.h"
#include "estimator_graphics/bins.h"
#include "estimator_graphics/waterfall.h"

#include "config/transcription.h"
#include "config/graphics.h"

#include <fftw3.h>

#include <vector>


class HighRes : public Estimator {
    public:
        HighRes(float *&input_buffer, int &buffer_size);
        ~HighRes() override;

        Estimators get_type() const override;

        void perform(float *const input_buffer, NoteEvents &note_events) override;


    private:
        // int in_size;
        float *in;
        fftwf_complex *out;
        fftwf_plan p;

        float window_func[FRAME_SIZE];
        double gaussian[KERNEL_WIDTH];

        double prev_power;


        void calc_envelope(const double norms[(FRAME_SIZE / 2) + 1], double envelope[(FRAME_SIZE / 2) + 1]);

        void all_max(const double norms[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks);
        void envelope_peaks(const double norms[(FRAME_SIZE / 2) + 1], const double envelope[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks);

        void min_dy_peaks(const double norms[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks);

        void interpolate_peaks(NoteSet &noteset, const double norms[(FRAME_SIZE / 2) + 1], const std::vector<int> &peaks);

        void get_loudest_peak(NoteSet &out_notes, const NoteSet &candidate_notes);
        void get_lowest_peak(NoteSet &out_notes, const NoteSet &candidate_notes);
        void get_likeliest_note(NoteSet &out_notes, const NoteSet &candidate_notes);
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
            }
        };

        // These should only be called by the HighRes estimator in perform()
        Spectrum &get_spectrum() {return spectrum;};
        Spectrum &get_envelope() {return envelope;};
        std::vector<double> &get_peaks() {return peak_frequencies;};


    private:
        Spectrogram spectrogram;
        Bins bins;
        Waterfall waterfall;

        // These get set during a perform() call
        Spectrum spectrum;
        Spectrum envelope;
        std::vector<double> peak_frequencies;
};


#endif  // DIGISTRING_ESTIMATORS_HIGHRES_H
