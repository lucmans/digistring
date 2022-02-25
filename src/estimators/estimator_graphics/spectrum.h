#ifndef DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_SPECTRUM_H
#define DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_SPECTRUM_H


#include <vector>


struct Data {
    double freq;  // Hz
    double amp;
    double bin_size;  // Hz; total width (both below and above freq together)

    Data(const double f, const double a, const double b) {
        freq = f;
        amp = a;
        bin_size = b;
    }
};


typedef std::vector<Data> SpectrumData;

class Spectrum {
    public:
        Spectrum();
        ~Spectrum();

        void add_data(const double freq, const double amp, const double bin_size);

        const SpectrumData &get_data() const;
        int get_size() const;

        void clear();

        // Sorts data vector on frequency
        void sort();


    private:
        SpectrumData spectrum;
};


#endif  // DIGISTRING_ESTIMATORS_ESTIMATOR_GRAPHICS_SPECTRUM_H
