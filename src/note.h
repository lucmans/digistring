
#ifndef NOTE_H
#define NOTE_H


#include <ostream>


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

    Note(const double freq, const double amp);
    Note(const Notes note, const int octave);
};

std::ostream& operator<<(std::ostream &s, const Note &note);


// Throws std::string with error description on invalid strings
// A valid note_string consists of a note name (A-G), with optional accidental ('#' or 'd' for sharp, 'b' for flat) and an integer denoting the octave
Note string_to_note(const std::string &note_string);


#endif  // NOTE_H
