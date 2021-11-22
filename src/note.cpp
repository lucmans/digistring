
#include "note.h"

#include "config.h"
#include "error.h"

#include <cmath>
#include <string>
#include <ostream>


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


Note::Note(const double _freq, const double _amp) {
    freq = _freq;
    amp = _amp;

    constexpr const double C0 = A4 * exp2(-57.0 / 12.0);
    const int note_distance = (12.0 * log2(freq / C0)) + 0.5;

    note = static_cast<Notes>((note_distance + 12) % 12);
    octave = note_distance / 12;

    const double tuned = C0 * exp2((double)octave + (double)((double)note / 12.0));
    error = 1200.0 * log2(freq / tuned);
}

Note::Note(const Notes _note, const int _octave) {
    note = _note;
    octave = _octave;

    constexpr const double C0 = A4 * exp2(-57.0 / 12.0);
    freq = C0 * exp2((double)octave + (double)((double)note / 12.0));
    amp = -1.0;
    error = 0.0;
}


std::ostream& operator<<(std::ostream &s, const Note &note) {
    s << note_string[static_cast<int>(note.note)] << stringify_sub(note.octave);
    // s << note_string[static_cast<int>(note.note)] << stringify_sub(note.octave) << " " << note.freq << " " << note.error;
    return s;
}


Note string_to_note(const std::string &note_string) {
    if(note_string.size() < 2)
        throw(std::string("String too short"));

    int note_distance, octave_distance;
    int modifier = 0;
    if(note_string[1] == '#' || note_string[1] == 'd')
        modifier = 1;
    else if(note_string[1] == 'b')
        modifier = -1;

    // note_distance = 0;
    // switch(note_string[0]) {
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
    //         throw(std::string("Incorrect note name ('" + note_string[0] + "')"));
    // }

    switch(note_string[0]) {
        case 'b': case 'B':
            note_distance = static_cast<int>(Notes::B);
            break;

        case 'a': case 'A':
            note_distance = static_cast<int>(Notes::A);
            break;

        case 'g': case 'G':
            note_distance = static_cast<int>(Notes::G);
            break;

        case 'f': case 'F':
            note_distance = static_cast<int>(Notes::F);
            break;

        case 'e': case 'E':
            note_distance = static_cast<int>(Notes::E);
            break;

        case 'd': case 'D':
            note_distance = static_cast<int>(Notes::D);
            break;

        case 'c': case 'C':
            note_distance = static_cast<int>(Notes::C);
            break;

        default:
            throw(std::string("Incorrect note name '" + note_string.substr(0, 1) + "'"));
    }

    note_distance += modifier;
    note_distance = (note_distance + 12) % 12;

    try {
        if(modifier == 0)
            octave_distance = std::stoi(note_string.substr(1));
        else
            octave_distance = std::stoi(note_string.substr(2));
    }
    catch(...) {
        throw(std::string("Incorrect octave number '" + (modifier == 0 ? note_string.substr(1) : note_string.substr(2)) + "'"));
    }

    return Note(static_cast<Notes>(note_distance), octave_distance);
}
