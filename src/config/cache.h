#ifndef DIGISTRING_CONFIG_CACHE_H
#define DIGISTRING_CONFIG_CACHE_H


#include <string>


// Cache directory relative from resource directory
const std::string CACHE_DIR_FROM_RSC_DIR = "../cache/";

// The last % is replaced by the length of the window
// The last $ is replaces by the db attenuation
const std::string DOLPH_WINDOW_FILENAME = "dolph_window_%_$.txt";


#endif  // DIGISTRING_CONFIG_CACHE_H
