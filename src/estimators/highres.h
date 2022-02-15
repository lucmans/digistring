
#ifndef HIGHRES_H
#define HIGHRES_H


#include "estimator.h"

#include "config/transcription.h"

#include <fftw3.h>

#include <vector>


// Gaussian average settings (for peak picking)
const int KERNEL_WIDTH = 47;  // Choose odd value
const int MID = KERNEL_WIDTH / 2;

const double SIGMA = 1.2;  // Higher values of sigma make values close to kernel center weight more


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


        void calc_envelope(const double norms[(FRAME_SIZE / 2) + 1], double envelope[(FRAME_SIZE / 2) + 1]);

        void all_max(const double norms[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks);
        void envelope_peaks(const double norms[(FRAME_SIZE / 2) + 1], const double envelope[(FRAME_SIZE / 2) + 1], std::vector<int> &peaks);

        void interpolate_peaks(NoteSet &noteset, const double norms[(FRAME_SIZE / 2) + 1], const std::vector<int> &peaks);

        void get_loudest_peak(NoteSet &out_notes, const NoteSet &candidate_notes);
        void get_lowest_peak(NoteSet &out_notes, const NoteSet &candidate_notes);
        void get_likeliest_note(NoteSet &out_notes, const NoteSet &candidate_notes);
};


#endif  // HIGHRES_H
