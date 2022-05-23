#ifndef DIGISTRING_SAMPLE_GETTER_SAMPLE_GETTER_H
#define DIGISTRING_SAMPLE_GETTER_SAMPLE_GETTER_H


#include "config/transcription.h"

#include <map>
#include <string>


/* When adding a new sample getter, don't forget to include the file in sample_getters.h */

// Different sample getter types
enum class SampleGetters {
    audio_file, audio_in, wave_generator, note_generator, increment
};

// For printing enum
// If a string isn't present in SampleGetterString for every type, random crashes may happen
const std::map<const SampleGetters, const std::string> SampleGetterString = {
    {SampleGetters::audio_file, "audio file"},
    {SampleGetters::audio_in, "audio in"},
    {SampleGetters::wave_generator, "wave generator"},
    {SampleGetters::note_generator, "note generator"},
    {SampleGetters::increment, "increment (debug)"}
};


class SampleGetter {
    public:
        SampleGetter(const int input_buffer_size);
        virtual ~SampleGetter();

        // This function should only return its type as named in SampleGetters
        virtual SampleGetters get_type() const = 0;

        // Returns true when getting samples from real sample source (e.g. audio recording device)
        // Shows if getting samples may block, which implies it should be the limiting factor of program loop
        // In other words, don't perform any audio syncing in Program if a SampleGetter blocks
        // This helps with minimizing the latency of Digistring on blocking SampleGetters
        virtual bool is_audio_recording_device() const {return false;};

        long get_played_samples() const;
        double get_played_time() const;  // In seconds

        // These methods might not exist for all sample getter types, so define empty
        virtual void pitch_up() {};
        virtual void pitch_down() {};

        // Has to increment played_samples
        // Returns number of newly read samples (which may be less than n_samples due to input buffer overlapping)
        virtual int get_frame(float *const in, const int n_samples) = 0;

        /* Overlap function
         * Note that n_samples has to be the same every call for overlapping to work!
         * Furthermore, n_samples should never exceed overlap_buffer_size, given on construction */
        // Pastes the overlapping part of previous frame and sets 'in' to new start and sets n_samples to remaining space
        // By changing the passed arguments, the caller can continue working with in and n_samples as if nothing happened
        void calc_and_paste_overlap(float *&in, int &n_samples) const;

        // Copy the part of in that will overlap with the next frame (end) to the overlap_buffer
        void copy_overlap(float *const in, const int n_samples);


    protected:
        long played_samples;

        // Only available if DO_OVERLAP or DO_OVERLAP_NON_BLOCK
        float *overlap_buffer;
        int overlap_buffer_size;
};


#endif  // DIGISTRING_SAMPLE_GETTER_SAMPLE_GETTER_H
