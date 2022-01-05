
#ifndef HIGHRES_H
#define HIGHRES_H


#include "estimator.h"

#include "../config.h"

#include <fftw3.h>

#include <vector>


// Gaussian average settings (for peak picking)
const int KERNEL_WIDTH = 47;  // Choose odd value
const int MID = KERNEL_WIDTH / 2;

const double SIGMA = 1.2;  // Higher values of sigma make values close to kernel center weight more


class HighRes : public Estimator {
    public:
        HighRes(float *const input_buffer);
        ~HighRes();

        Estimators get_type() const override;

        // The input buffer has to be freed by caller using fftwf_free()
        static float *create_input_buffer(int &buffer_size);
        float *_create_input_buffer(int &buffer_size) const override;

        // Implemented by superclass
        // void free_input_buffer(float *const input_buffer) const;

        void perform(float *const input_buffer, NoteSet &noteset) override;


    private:
        fftwf_complex *out;
        fftwf_plan p;

        double window_func[FRAME_SIZE];
        double gaussian[KERNEL_WIDTH];


        void calc_envelope(const double norms[(FRAME_SIZE / 2) + 1], double envelope[(FRAME_SIZE / 2) + 1]);

        void all_max(const double norms[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks);
        void envelope_peaks(const double norms[(FRAME_SIZE / 2) + 1], const double envelope[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks);

        void interpolate_peaks(NoteSet &noteset, const double norms[(FRAME_SIZE / 2) + 1], const std::vector<int> &peaks);

        void get_loudest_peak(NoteSet &out_notes, const NoteSet &candidate_notes);
        void get_lowest_peak(NoteSet &out_notes, const NoteSet &candidate_notes);
        void get_likeliest_note(NoteSet &out_notes, const NoteSet &candidate_notes);
};


#endif  // HIGHRES_H
