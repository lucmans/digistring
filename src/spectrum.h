
#ifndef SPECTRUM_H
#define SPECTRUM_H


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

        const SpectrumData *get_data() const;
        int get_size() const;

        void clear();

        // Sorts data vector on frequency
        void sort();

        // Allows range based for loops for Spectrum
        // SpectrumData::iterator begin() const;
        // SpectrumData::iterator end() const;
        SpectrumData::const_iterator cbegin() const;
        SpectrumData::const_iterator cend() const;


    private:
        SpectrumData spectrum;
};


#endif  // SPECTRUM_H
