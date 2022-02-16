
#ifndef DATA_CACHE
#define DATA_CACHE


#include <string>


class DataCache {
    public:
        // Should be called once at start of program after parsing CLI arguments and verifying resource directory
        static void init_cache();

        static const std::string get_cache_dir();

        // The attenuation in the filename is rounded to three decimal digits
        static std::string get_dolph_filename(const int size, const double attenuation);
        static std::string get_dolph_path();  // For adding subdirectories in the future

        static void save_dolph_window(const double in[], const int size, const double attenuation);
        static void save_dolph_window(const float in[], const int size, const double attenuation);
        static bool load_dolph_window(double out[], const int size, const double attenuation);
        static bool load_dolph_window(float out[], const int size, const double attenuation);


    private:
        // Remove ability to create an instance
        DataCache() {}

        // Is set in init_cache()
        static std::string cache_dir;
};


#endif  // DATA_CACHE
