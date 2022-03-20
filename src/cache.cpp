#include "cache.h"

#include <error.h>

#include <config/cache.h>
#include <config/cli_args.h>

#include <string>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <sstream>


// Is set in init_cache()
std::string Cache::cache_dir = "";


void Cache::init_cache() {
    if(cache_dir != "") {
        warning("Cache directory is already initialized; this function shouldn't be called twice...");
        return;
    }

    std::filesystem::path path(cli_args.rsc_dir + CACHE_DIR_FROM_RSC_DIR + '/');
    path = path.lexically_normal();
    cache_dir = path.string();

    // If the string to std::filesystem::exists() ends in a /, no files are found
    const std::string cache_file = cache_dir.substr(0, cache_dir.size() - 1);
    if(!std::filesystem::exists(cache_file)) {
        try {
            std::filesystem::create_directory(cache_dir);
        }
        catch(const std::filesystem::filesystem_error &e) {
            error("Failed to create cache directory '" + cache_dir + "'\nOS error: " + e.code().message());
            exit(EXIT_FAILURE);
        }
    }

    if(!std::filesystem::is_directory(cache_dir)) {
        error("Cache path is not a directory; please remove the file at '" + cache_dir + "'");
        exit(EXIT_FAILURE);
    }
}


const std::string Cache::get_cache_dir() {
    if(cache_dir == "") {
        warning("Cache was not yet initialized, so directory is not yet set");
        return "";
    }

    return cache_dir;
}


std::string Cache::get_dolph_filename(const int size, const double attenuation) {
    // Copy the const string, as replace() mutates string
    std::string out = DOLPH_WINDOW_FILENAME;

    // Put size in filename
    size_t pos = out.find_last_of('%');
    if(pos == std::string::npos) {
        error("No '%' in DOLPH_WINDOW_FILENAME in data cache config");
        exit(EXIT_FAILURE);
    }
    out.replace(pos, 1, std::to_string(size));

    // Put attenuation in filename
    pos = out.find_last_of('$');
    if(pos == std::string::npos) {
        error("No '$' in DOLPH_WINDOW_FILENAME in data cache config");
        exit(EXIT_FAILURE);
    }

    // Only use 3 decimal digits for the filename
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << attenuation;
    out.replace(pos, 1, ss.str());

    return out;
}


std::string Cache::get_dolph_path() {
    return cache_dir;
}


void Cache::save_dolph_window(const double in[], const int size, const double attenuation) {
    if constexpr(DISABLE_CACHE)
        return;

    const std::string filename = get_dolph_filename(size, attenuation);

    std::fstream dolph_file(cache_dir + filename, std::ios::out);
    if(!dolph_file.is_open()) {
        warning("Failed to open Dolph Chebyshev window cache file '" + filename + "' for writing; not saving to cache");
        return;
    }

    for(int i = 0; i < size; i++)
        dolph_file << in[i] << '\n';
    dolph_file << std::endl;
}

void Cache::save_dolph_window(const float in[], const int size, const double attenuation) {
    if constexpr(DISABLE_CACHE)
        return;

    const std::string filename = get_dolph_filename(size, attenuation);

    std::fstream dolph_file(cache_dir + filename, std::ios::out);
    if(!dolph_file.is_open()) {
        warning("Failed to open Dolph Chebyshev window cache file '" + filename + "' for writing; not saving to cache");
        return;
    }

    for(int i = 0; i < size; i++)
        dolph_file << in[i] << '\n';
    dolph_file << std::endl;
}


bool Cache::load_dolph_window(double out[], const int size, const double attenuation) {
    if constexpr(DISABLE_CACHE)
        return false;

    const std::string filename = get_dolph_filename(size, attenuation);
    if(!std::filesystem::exists(cache_dir + filename))
        return false;

    std::fstream dolph_file(cache_dir + filename);
    for(int i = 0; i < size; i++)
        dolph_file >> out[i];

    return true;
}

bool Cache::load_dolph_window(float out[], const int size, const double attenuation) {
    if constexpr(DISABLE_CACHE)
        return false;

    const std::string filename = get_dolph_filename(size, attenuation);
    if(!std::filesystem::exists(cache_dir + filename))
        return false;

    std::fstream dolph_file(cache_dir + filename);
    for(int i = 0; i < size; i++)
        dolph_file >> out[i];

    return true;
}
