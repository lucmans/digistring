#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <iostream>
#include <chrono>


// Config
const int points = 800;  // Set this lower on seg faults (stack overflow)
const int max_n = 5000;  // Higher max n is more calculations

// Real min and max followed by imaginary min and max
const double dbounds[4] = {-2.0, 1.0, -1.5, 1.5};
const float fbounds[4] = {(float)dbounds[0], (float)dbounds[1], (float)dbounds[2], (float)dbounds[3]};
const double dp = (dbounds[1] - dbounds[0]) / (double)points;
const float df = (fbounds[1] - fbounds[0]) / (float)points;


int main(int argc, char *argv[]) {
    // Arrays contain escape iteration number
    // If a number is equal to max_n, it is considered in the Mandelbrot set
    // For real Mandelbrot set, max_n is infinity
    // Volatile so it doesn't get optimized away (e.g. when compiling with -O3 and no result check)
    volatile int output_w[points][points] = {};
    volatile int output_f[points][points] = {};
    volatile int output_d[points][points] = {};

    std::chrono::steady_clock::time_point start, stop;
    std::chrono::duration<double> dur;

    // Warm up
    std::cout << "Warming up (same as double)..." << std::endl;
    start = std::chrono::steady_clock::now();
    for(int i = 0; i < points; i++) {
        for(int j = 0; j < points; j++) {
            double z[2] = {0.0, 0.0};
            double z_squared[2] = {0.0, 0.0};
            const double c[2] = {dbounds[0] + ((double)i * dp), dbounds[3] - ((double)j * dp)};
            
            int n;
            for(n = 0; n < max_n && z_squared[0] + z_squared[1] <= 4.0; n++) {
                z[1] = z[0] * z[1] * 2.0;
                z[0] = z_squared[0] - z_squared[1];

                z[0] += c[0];
                z[1] += c[1];

                z_squared[0] = z[0] * z[0];
                z_squared[1] = z[1] * z[1];
            }

            output_w[i][j] = n;
        }
    }
    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "Warmed up in " << dur.count() << " seconds" << std::endl;
    std::cout << std::endl;

    std::cout << "Floats" << std::endl;
    start = std::chrono::steady_clock::now();
    for(int i = 0; i < points; i++) {
        for(int j = 0; j < points; j++) {
            float z[2] = {0.0f, 0.0f};
            float z_squared[2] = {0.0f, 0.0f};
            const float c[2] = {fbounds[0] + ((float)i * df), fbounds[3] - ((float)j * df)};
            
            int n;
            for(n = 0; n < max_n && z_squared[0] + z_squared[1] <= 4.0f; n++) {
                z[1] = z[0] * z[1] * 2.0f;
                z[0] = z_squared[0] - z_squared[1];

                z[0] += c[0];
                z[1] += c[1];

                z_squared[0] = z[0] * z[0];
                z_squared[1] = z[1] * z[1];
            }

            output_f[i][j] = n;
        }
    }
    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "Float done in " << dur.count() << " seconds" << std::endl;
    std::cout << std::endl;

    std::cout << "Doubles" << std::endl;
    start = std::chrono::steady_clock::now();
    for(int i = 0; i < points; i++) {
        for(int j = 0; j < points; j++) {
            double z[2] = {0.0, 0.0};
            double z_squared[2] = {0.0, 0.0};
            const double c[2] = {dbounds[0] + ((double)i * dp), dbounds[3] - ((double)j * dp)};
            
            int n;
            for(n = 0; n < max_n && z_squared[0] + z_squared[1] <= 4.0; n++) {
                z[1] = z[0] * z[1] * 2.0;
                z[0] = z_squared[0] - z_squared[1];

                z[0] += c[0];
                z[1] += c[1];

                z_squared[0] = z[0] * z[0];
                z_squared[1] = z[1] * z[1];
            }

            output_d[i][j] = n;
        }
    }
    stop = std::chrono::steady_clock::now();
    dur = stop - start;
    std::cout << "Doubles done in " << dur.count() << " seconds" << std::endl;
    std::cout << std::endl;

    // Check if the set is the same
    // Also prevent optimizing calculation of outputs away
    int errors = 0;
    for(int i = 0; i < points; i++) {
        for(int j = 0; j < points; j++) {
            if((float)output_d[i][j] != output_f[i][j])
                errors++;
        }
    }
    std::cout << errors << '/' << points * points << " pixels different between float and double set" << std::endl;

    return EXIT_SUCCESS;
}
