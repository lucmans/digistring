
#include "spectrum.h"

#include <vector>
#include <algorithm>  // std::sort()


Spectrum::Spectrum() {

}

Spectrum::~Spectrum() {

}


void Spectrum::add_data(const double freq, const double amp, const double bin_size) {
    spectrum.push_back(Data(freq, amp, bin_size));
}


// void Spectrum::add_envelope(const double freq, const double amp) {
//     env.push_back(EnvData(freq, amp));
// }

// EnvelopeData *Spectrum::get_envelope() {
//     return &env;
// }


const SpectrumData *Spectrum::get_data() const {
    return &spectrum;
}

int Spectrum::get_size() const {
    return spectrum.size();
}

// const EnvelopeData *Spectrum::get_env() const {
//     return &env;
// }


void Spectrum::clear() {
    spectrum.clear();
}


bool sort_freq(const Data &a, const Data &b) {
    return a.freq < b.freq;
}

void Spectrum::sort() {
    std::sort(spectrum.begin(), spectrum.end(), sort_freq);
}


// SpectrumData::iterator Spectrum::begin() const {
//     return spectrum.begin();
// }

// SpectrumData::iterator Spectrum::end() const {
//     return spectrum.end();
// }

// SpectrumData::const_iterator Spectrum::cbegin() const {
//     return spectrum.cbegin();
// }

// SpectrumData::const_iterator Spectrum::cend() const {
//     return spectrum.cend();
// }
