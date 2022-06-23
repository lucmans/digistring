#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <chrono>

#include <cstring>
#include <cmath>


// Config
const int size = 1<<14;  // Set this lower on seg faults (stack overflow)
const int cycles = 10000000;  // Higher max n is more calculations

const double FREQ = 1000.0;


int main(int argc, char *argv[]) {
    // Volatile so it doesn't get optimized away (e.g. when compiling with -O3 and no result check)
    float buf1[size] = {};
    float buf2[size] = {};

    if(sizeof(float) != 4)
        std::cout << "Warning: Size of float is not 32 bits, so times might not reflect sample copy times" << std::endl;

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::duration<double> dur;

    std::cout << "Filling array with " << size << " elements with sine wave..." << std::endl;
    start = std::chrono::steady_clock::now();

    for(int i = 0; i < size; i++)
        buf1[i] = sin((2.0f * (float)M_PI * (float)i * FREQ) / (float)size);

    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "filled array in " << dur.count() << " seconds" << std::endl;
    std::cout << std::endl;


    std::cout << "memcpy..." << std::endl;
    start = std::chrono::steady_clock::now();

    for(int i = 0; i < cycles; i++)
        memcpy(buf1, buf2, size * sizeof(float));

    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "memcpy() done in " << dur.count() << " seconds" << std::endl;
    std::cout << 1000.0 * dur.count() / (double)cycles << " milliseconds per cycle" << std::endl;
    std::cout << std::endl;


    std::cout << "memmove..." << std::endl;
    start = std::chrono::steady_clock::now();

    for(int i = 0; i < cycles; i++)
        memmove(buf2, buf1, size * sizeof(float));

    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "memmove() done in " << dur.count() << " seconds" << std::endl;
    std::cout << 1000.0 * dur.count() / (double)cycles << " milliseconds per cycle" << std::endl;
    std::cout << std::endl;

    // Prevent memcpys from being optimized away
    double total = 0.0;
    for(int i = 0; i < size; i++)
        total += buf1[i] + buf2[i];
    std::cout << "Total value in buffers (to prevent memcpy from being optimized away) " << total << std::endl;

    return EXIT_SUCCESS;
}
