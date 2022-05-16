#include "note.h"

#include "error.h"

#include <cmath>  // log10()
#include <string>
#include <ostream>
#include <iomanip>  // std::setw()
#include <algorithm>  // std::max()
#include <vector>
#include <stdexcept>  // std::runtime_error


const char *note_string[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
const char *sub[10] = {"\xe2\x82\x80", "\xe2\x82\x81", "\xe2\x82\x82",
                       "\xe2\x82\x83", "\xe2\x82\x84", "\xe2\x82\x85",
                       "\xe2\x82\x86", "\xe2\x82\x87", "\xe2\x82\x88", "\xe2\x82\x89"};

// Makes a subscript number string of n
const std::string stringify_sub(int n) {
    std::string sign = "";

    if(n < 0) {
        sign = "-";
        n *= -1;
    }

    if(n < 10)
        return sign + sub[n];

    std::string out;
    while(n != 0) {
        out = sub[n % 10] + out;
        n /= 10;
    }

    return sign + out;
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

    s << noteset[0];
    for(unsigned int i = 1; i < noteset.size(); i++)
        s << ' ' << noteset[i];

    return s;
}


// Returns the number of printed character for a subscript
// Needed as subscript uses wide characters, which count as two
int subscript_offset(const int subscript) {
    if(subscript == 0)
        return 1;

    else if(subscript < 0)
        return log10(-subscript) + 2;

    return log10(subscript) + 1;
}

void print_overtones(const Note &note, const int n_overtones, const bool print_midi_number/* = false*/) {
    const std::string n_txt = "n",
                      f_harmonic_txt = "f_harmonic",
                      closest_note_txt = "closest note",
                      f_closest_txt = "f_closest",
                      cent_error_txt = "cent error",
                      midi_number_txt = "midi number";

    const int precision = 3;
    const int spacing = 2;
    const Note max_note = Note(note.freq * (double)n_overtones);
    const std::vector<int> column_width = {
        std::max({
            (int)n_txt.size(),
            (int)log10(n_overtones - 1) + 1
        }),
        std::max({
            (int)f_harmonic_txt.size(),
            (int)log10(note.freq * n_overtones) + 1 + 1 + precision  // +1 for decimal point
        }) + spacing,
        std::max({
            (int)closest_note_txt.size(),
            1 + 1 + subscript_offset(max_note.octave)  // Note name + accidental + subscript
        }) + spacing,
        std::max({
            (int)f_closest_txt.size(),
            (int)log10(A4 * exp2((double)(max_note.midi_number - 69) / 12.0)) + 1 + 1 + precision
        }) + spacing,
        std::max({
            (int)cent_error_txt.size(),
            1 + 2 + 1 + precision  // minus + max 50 + decimal point + precision
        }) + spacing,
        std::max({
            (int)midi_number_txt.size(),
            (int)log10(max_note.midi_number) + 1
        }) + spacing
    };

    // Print header
    std::cout << std::right
              << std::setw(column_width[0]) << n_txt
              << std::setw(column_width[1]) << f_harmonic_txt
              << std::setw(column_width[2]) << closest_note_txt
              << std::setw(column_width[3]) << f_closest_txt
              << std::setw(column_width[4]) << cent_error_txt;
    if(print_midi_number)
        std::cout << std::setw(column_width[5]) << midi_number_txt;
    std::cout << std::endl;

    std::cout << std::fixed << std::setprecision(precision);
    for(int i = 1; i <= n_overtones; i++) {
        Note overtone(note.freq * (double)i);
        std::cout << std::setw(column_width[0])                                     << i - 1
                  << std::setw(column_width[1])                                     << overtone.freq
                  << std::setw(column_width[2] - subscript_offset(overtone.octave)) << overtone
                  << std::setw(column_width[3])                                     << A4 * exp2((double)(overtone.midi_number - 69) / 12.0)
                  << std::setw(column_width[4])                                     << overtone.error;
        if(print_midi_number)
            std::cout << std::setw(column_width[5])  << overtone.midi_number;
        std::cout << std::endl;
    }
}


Note string_to_note(const std::string &in_string) {
    if(in_string.size() < 2)
        throw(std::runtime_error("String too short; need note and octave number"));

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
    catch(const std::out_of_range &e) {
        throw(std::runtime_error("Octave number too large to store in an integer"));
    }
    catch(const std::exception &e) {
        throw(std::runtime_error("Incorrect octave number '" + (modifier == 0 ? in_string.substr(1) : in_string.substr(2)) + "'"));
    }

    return Note(static_cast<Notes>(note_distance), octave_distance);
}
