#include "note.h"

#include "error.h"

#include <cmath>  // log10()
#include <string>
#include <ostream>
#include <iomanip>  // std::setw()
#include <algorithm>  // std::max()
#include <stdexcept>  // std::runtime_error


const char *note_string[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
const char *sub[10] = {"\xe2\x82\x80", "\xe2\x82\x81", "\xe2\x82\x82",
                       "\xe2\x82\x83", "\xe2\x82\x84", "\xe2\x82\x85",
                       "\xe2\x82\x86", "\xe2\x82\x87", "\xe2\x82\x88", "\xe2\x82\x89"};

// Makes a subscript number string of n
const std::string stringify_sub(int n) {
    std::string out;
    if(n < 0) {
        out = "-";
        n *= -1;
    }

    if(n < 10)
        return out + sub[n];

    while(n != 0) {
        out = sub[n % 10] + out;
        n /= 10;
    }

    return out;
}


// Note::Note(const double _freq, const double _amp) {
//     freq = _freq;
//     amp = _amp;

//     constexpr double C0 = A4 * exp2(-57.0 / 12.0);
//     const int note_distance = (int)round(12.0 * log2(freq / C0));  // semitones from C0

//     // First modulo to get a number within -12 < n < 12, then add 12 and another modulo to get 0 <= n < 12
//     note = static_cast<Notes>(((note_distance % 12) + 12) % 12);
//     octave = floor((double)note_distance / 12.0);

//     const double tuned = C0 * exp2((double)octave + (static_cast<double>(note) / 12.0));
//     error = 1200.0 * log2(freq / tuned);

//     midi_number = 12 + note_distance;
// }

// Note::Note(const Notes _note, const int _octave) {
//     note = _note;
//     octave = _octave;

//     constexpr double C0 = A4 * exp2(-57.0 / 12.0);
//     freq = C0 * exp2((double)octave + (static_cast<double>(note) / 12.0));
//     amp = -1.0;
//     error = 0.0;

//     midi_number = 12 + static_cast<int>(_note) + (_octave * 12)
// }

// Note::Note(const int _midi_number) {
//     midi_number = _midi_number;

//     const int note_distance = midi_number - 12;  // semitones from C0

//     freq = C0 * exp2((double)note_distance / 12.0);
//     amp = -1.0;
//     error = 0.0;

//     note = static_cast<Notes>(((note_distance % 12) + 12) % 12);
//     octave = floor((double)note_distance / 12.0);
// }


std::ostream& operator<<(std::ostream &s, const Note &note) {
    s << note_string[static_cast<int>(note.note)] << stringify_sub(note.octave);
    // s << note_string[static_cast<int>(note.note)] << stringify_sub(note.octave) << " " << note.freq << " " << note.error;
    return s;
}

std::string note_to_string(const Note &note) {
    return note_string[static_cast<int>(note.note)] + stringify_sub(note.octave);
}

std::string note_to_string_ascii(const Note &note) {
    return note_string[static_cast<int>(note.note)] + std::to_string(note.octave);
}


std::ostream& operator<<(std::ostream &s, const NoteSet &noteset) {
    if(noteset.size() == 0)
        return s;

    std::cout << noteset[0];
    for(unsigned int i = 1; i < noteset.size(); i++)
        s << ' ' << noteset[i];

    return s;
}


int subscript_offset(const int subscript) {
    if(subscript == 0)
        return 1;

    else if(subscript < 0)
        return log10(-subscript) + 2;

    return log10(subscript) + 1;
}

void print_overtones(const Note &note, const int n_overtones) {
    // Calculate column sizes
    const int max_n_width = log10(n_overtones - 1) + 1;
    const int max_harm_width = std::max((int)log10(note.freq * n_overtones) + 7, 12);
    const int max_closest_width = std::max((int)log10(note.freq * n_overtones) + 7, 11);

    // Print header
    std::cout << std::right
              << std::setw(max_n_width)  << "n"
              << std::setw(max_harm_width) << "f_harmonic"
              << std::setw(14) << "closest note"
              << std::setw(max_closest_width) << "f_closest"
              << std::setw(12) << "cent error" << std::endl;
              // << std::setw(12) << "cent error"
              // << std::setw(13) << "midi number" << std::endl;

    for(int i = 1; i <= n_overtones; i++) {
        Note overtone(note.freq * i, 0);
        std::cout << std::fixed << std::setprecision(3)
                  << std::setw(max_n_width) << i - 1
                  << std::setw(max_harm_width) << overtone.freq
                  << std::setw(14 - subscript_offset(overtone.octave)) << overtone
                  << std::setw(max_closest_width) << A4 * exp2(round(12.0 * log2((overtone.freq) / A4)) / 12.0)
                  << std::setw(12) << overtone.error << std::endl;
                  // << std::setw(12) << overtone.error
                  // << std::setw(13) << note.midi_number << std::endl;
    }
    std::cout << std::endl;
}


Note string_to_note(const std::string &in_string) {
    if(in_string.size() < 2)
        throw(std::runtime_error("String too short"));

    int note_distance, octave_distance;
    int modifier = 0;
    if(in_string[1] == '#' || in_string[1] == 'd')
        modifier = 1;
    else if(in_string[1] == 'b')
        modifier = -1;

    // note_distance = 0;
    // switch(in_string[0]) {
    //     case 'b': case 'B':
    //         note_distance += 2;
    //         __attribute__ ((fallthrough));
    //     case 'a': case 'A':
    //         note_distance += 2;
    //         __attribute__ ((fallthrough));
    //     case 'g': case 'G':
    //         note_distance += 2;
    //         __attribute__ ((fallthrough));
    //     case 'f': case 'F':
    //         note_distance += 1;
    //         __attribute__ ((fallthrough));
    //     case 'e': case 'E':
    //         note_distance += 2;
    //         __attribute__ ((fallthrough));
    //     case 'd': case 'D':
    //         note_distance += 2;
    //         __attribute__ ((fallthrough));
    //     case 'c': case 'C':
    //         break;

    //     default:
    //         throw(std::runtime_error("Incorrect note name ('" + in_string[0] + "')"));
    // }

    switch(in_string[0]) {
        case 'c': case 'C':
            note_distance = static_cast<int>(Notes::C);
            break;

        case 'd': case 'D':
            note_distance = static_cast<int>(Notes::D);
            break;

        case 'e': case 'E':
            note_distance = static_cast<int>(Notes::E);
            break;

        case 'f': case 'F':
            note_distance = static_cast<int>(Notes::F);
            break;

        case 'g': case 'G':
            note_distance = static_cast<int>(Notes::G);
            break;

        case 'a': case 'A':
            note_distance = static_cast<int>(Notes::A);
            break;

        case 'b': case 'B':
            note_distance = static_cast<int>(Notes::B);
            break;

        default:
            throw(std::runtime_error("Incorrect note name '" + in_string.substr(0, 1) + "'"));
    }

    note_distance += modifier;
    note_distance = (note_distance + 12) % 12;

    try {
        if(modifier == 0)
            octave_distance = std::stoi(in_string.substr(1));
        else
            octave_distance = std::stoi(in_string.substr(2));
    }
    catch(...) {
        throw(std::runtime_error("Incorrect octave number '" + (modifier == 0 ? in_string.substr(1) : in_string.substr(2)) + "'"));
    }

    return Note(static_cast<Notes>(note_distance), octave_distance);
}
