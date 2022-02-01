
#ifndef NOTE_H
#define NOTE_H


#include <vector>
#include <string>
#include <ostream>
#include <cmath>


constexpr double A4 = 440.0;  // Hz
constexpr static double C0 = A4 * exp2(-57.0 / 12.0);  // Used for constexpr constructor


// 'd' denotes # (from diesis)
enum Notes : int {
    C  = 0, Cd = 1, D  = 2, Dd = 3, E  = 4,  F = 5,
    Fd = 6, G  = 7, Gd = 8, A  = 9, Ad = 10, B = 11
};


struct Note {
    double freq;
    double amp;

    // Closest note
    Notes note;
    int octave;
    double error;  // In cents, so between -50 and 50

    int midi_number;

    // Note(const double _freq, const double _amp);
    // Note(const Notes _note, const int _octave);  // If passing an integer as note, make sure it is 0 <= n < 12; otherwise, random crashes may occur
    // Note(const int _midi_number);

    // See original, non constexpr, constructors for more readable version (only difference is local variable substitution)
    constexpr Note(const double _freq, const double _amp) :
            freq(_freq),
            amp(_amp),
            note(static_cast<Notes>(((((int)round(12.0 * log2(_freq / C0))) % 12) + 12) % 12)),
            octave(floor((double)((int)round(12.0 * log2(_freq / C0))) / 12.0)),
            error(1200.0 * log2(_freq / (C0 * exp2((double)octave + (static_cast<double>(note) / 12.0))))),
            midi_number(12 + round(12.0 * log2(_freq / C0))) {};

    // If passing an integer as note, make sure it is 0 <= n < 12; otherwise, random crashes may occur
    constexpr Note(const Notes _note, const int _octave) :
            freq(C0 * exp2((double)_octave + (static_cast<double>(_note) / 12.0))),
            amp(-1.0),
            note(_note),
            octave(_octave),
            error(0.0),
            midi_number(12 + (_octave * 12) + static_cast<int>(_note)) {
        // static_assert(static_cast<int>(_note) >= 0 && static_cast<int>(_note) < 12);
    };

    constexpr Note(const int _midi_number) :
            freq(C0 * exp2((double)(_midi_number - 12) / 12.0)),
            amp(-1.0),
            note(static_cast<Notes>((((_midi_number - 12) % 12) + 12) % 12)),
            octave(floor((double)(_midi_number - 12) / 12.0)),
            error(0.0),
            midi_number(_midi_number) {};
};

std::ostream& operator<<(std::ostream &s, const Note &note);
std::string note_to_string(const Note &note);
std::string note_to_string_ascii(const Note &note);  // Octave number without lowercase


typedef std::vector<Note> NoteSet;

std::ostream& operator<<(std::ostream &s, const NoteSet &noteset);


void print_overtones(const Note &note, const int n_overtones);


// Throws std::string with error description on invalid strings
// A valid in_string consists of a note name (A-G), with optional accidental ('#' or 'd' for sharp, 'b' for flat) and an integer denoting the octave
Note string_to_note(const std::string &in_string);


#endif  // NOTE_H
