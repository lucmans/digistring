
#ifndef SAMPLE_GETTER_H
#define SAMPLE_GETTER_H


#include <map>
#include <string>


/* When adding a new sample getter, don't forget to include the file in sample_getters.h */

// Different sample getter types
enum class SampleGetters {
    audio_file, audio_in, wave_generator, note_generator
};

// For printing enum
// If a string isn't present in SampleGetterString for every type, random crashes may happen
// extern const std::map<const SampleGetters, const std::string> SampleGetterString;
const std::map<const SampleGetters, const std::string> SampleGetterString = {{SampleGetters::audio_file, "audio file"},
                                                                             {SampleGetters::audio_in, "audio in"},
                                                                             {SampleGetters::wave_generator, "wave generator"},
                                                                             {SampleGetters::note_generator, "note generator"}};


class SampleGetter {
    public:
        SampleGetter();
        virtual ~SampleGetter();

        // This function should only return its type as named in SampleGetters
        virtual SampleGetters get_type() const = 0;

        // These methods might not exist for all sample getter types
        virtual void pitch_up() {};
        virtual void pitch_down() {};

        virtual void get_frame(float *const in, const int n_samples) = 0;


    protected:
        //
};


#endif  // SAMPLE_GETTER_H
