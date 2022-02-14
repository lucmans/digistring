
#include "window_func.h"

#include <cmath>

// For Dolph Chebyshev window (calling Python code and reading the output file)
#include <string>
#include <filesystem>
#include <fstream>
#include <cstdlib>  // system(), EXIT_FAILURE

#include "../error.h"


void rectangle_window(double window[], const int size) {
    for(int i = 0; i < size; i++)
        window[i] = 1.0;
}


void hamming_window(double window[], const int size) {
    const double a0 = 25.0 / 46.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;

        window[i] = a0 - ((1.0 - a0) * cos((2.0 * x) / N));
    }
}


void hann_window(double window[], const int size) {
    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = sin(x / N) * sin(x / N);
    }
}


void blackman_window(double window[], const int size) {
    const double a0 = 7938.0 / 18608.0;
    const double a1 = 9240.0 / 18608.0;
    const double a2 = 1430.0 / 18608.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N));
    }
}


void nuttall_window(double window[], const int size) {
    const double a0 = 0.355768;
    const double a1 = 0.487396;
    const double a2 = 0.144232;
    const double a3 = 0.012604;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_nuttall_window(double window[], const int size) {
    const double a0 = 0.3635819;
    const double a1 = 0.4891775;
    const double a2 = 0.1365995;
    const double a3 = 0.0106411;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_harris_window(double window[], const int size) {
    const double a0 = 0.35875;
    const double a1 = 0.48829;
    const double a2 = 0.14128;
    const double a3 = 0.01168;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void flat_top_window(double window[], const int size) {
    const double a0 = 0.21557895;
    const double a1 = 0.41663158;
    const double a2 = 0.277263158;
    const double a3 = 0.083578947;
    const double a4 = 0.006947368;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N))
                       + (a4 * cos((8.0 * x) / N));
    }
}


void welch_window(double window[], const int size) {
    const double N = size;
    const double hN = (double)size / 2.0;
    for(int i = 0; i < N; i++) {
        const double t = (i - hN) / hN;
        window[i] = 1 - (t * t);
    }
}


void dolph_chebyshev_window(double window[], const int size, const double attenuation) {
    std::string filename = TMP_DOLPH_WIN_FILENAME;
    if(std::filesystem::exists(filename)) {
        error("File '" + filename + "' already exists\nPlease delete this file before using the Dolph Chebyshev window.");
        exit(EXIT_FAILURE);
    }

    const int ret = system(("tools/dolph_chebyshev_window/dolph_chebyshev_window " + filename + ' ' + std::to_string(size) + ' ' + std::to_string(attenuation)).c_str());
    if(ret != 0) {
        error("Failed to get dolph chebyshev window");
        exit(EXIT_FAILURE);
    }

    std::fstream output_file(filename);
    for(int i = 0; i < size; i++)
        output_file >> window[i];

    if(!std::filesystem::remove(filename)) {
        error("Failed to delete '" + filename + "' after computing the Dolph Chebyshev window\nPlease remove manually");
        exit(EXIT_FAILURE);
    }
}



void rectangle_window(float window[], const int size) {
    for(int i = 0; i < size; i++)
        window[i] = 1.0;
}


void hamming_window(float window[], const int size) {
    const double a0 = 25.0 / 46.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;

        window[i] = a0 - ((1.0 - a0) * cos((2.0 * x) / N));
    }
}


void hann_window(float window[], const int size) {
    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = sin(x / N) * sin(x / N);
    }
}


void blackman_window(float window[], const int size) {
    const double a0 = 7938.0 / 18608.0;
    const double a1 = 9240.0 / 18608.0;
    const double a2 = 1430.0 / 18608.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N));
    }
}


void nuttall_window(float window[], const int size) {
    const double a0 = 0.355768;
    const double a1 = 0.487396;
    const double a2 = 0.144232;
    const double a3 = 0.012604;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_nuttall_window(float window[], const int size) {
    const double a0 = 0.3635819;
    const double a1 = 0.4891775;
    const double a2 = 0.1365995;
    const double a3 = 0.0106411;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_harris_window(float window[], const int size) {
    const double a0 = 0.35875;
    const double a1 = 0.48829;
    const double a2 = 0.14128;
    const double a3 = 0.01168;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void flat_top_window(float window[], const int size) {
    const double a0 = 0.21557895;
    const double a1 = 0.41663158;
    const double a2 = 0.277263158;
    const double a3 = 0.083578947;
    const double a4 = 0.006947368;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N))
                       + (a4 * cos((8.0 * x) / N));
    }
}


void welch_window(float window[], const int size) {
    const double N = size;
    const double hN = (double)size / 2.0;
    for(int i = 0; i < N; i++) {
        const double t = (i - hN) / hN;
        window[i] = 1 - (t * t);
    }
}


void dolph_chebyshev_window(float window[], const int size, const double attenuation) {
    std::string filename = TMP_DOLPH_WIN_FILENAME;
    if(std::filesystem::exists(filename)) {
        error("File '" + filename + "' already exists\nPlease delete this file before using the Dolph Chebyshev window.");
        exit(EXIT_FAILURE);
    }

    const int ret = system(("tools/dolph_chebyshev_window/dolph_chebyshev_window " + filename + ' ' + std::to_string(size) + ' ' + std::to_string(attenuation)).c_str());
    if(ret != 0) {
        error("Failed to get dolph chebyshev window");
        exit(EXIT_FAILURE);
    }

    std::fstream output_file(filename);
    for(int i = 0; i < size; i++)
        output_file >> window[i];

    if(!std::filesystem::remove(filename)) {
        error("Failed to delete '" + filename + "' after computing the Dolph Chebyshev window\nPlease remove manually");
        exit(EXIT_FAILURE);
    }
}

// #include <fftw3.h>
// #include <algorithm>
// #include "../error.h"
// void dolph_chebyshev_window(float window[], const int size) {
//     // The Dolph-Chebyshev is defined in the frequency domain
//     fftwf_complex *f_win_in = (fftwf_complex*)fftwf_malloc(size * sizeof(fftwf_complex));
//     fftwf_complex *out = (fftwf_complex*)fftwf_malloc(size * sizeof(fftwf_complex));
//     fftwf_plan p = fftwf_plan_dft_1d(size, f_win_in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

//     // Alpha
//     const double dB = -60.0;
//     const double alpha = dB / -20.0;
//     const double beta = cosh((1.0 / (double)size) * acosh(pow(10.0, alpha)));

//     info("dB: " + STR(dB));
//     info("alpha: " + STR(alpha));
//     info("beta: " + STR(beta));
//     info("");

//     for(int i = 0; i < size; i++) {
//         info("acos: " + STR(beta * cos((M_PI * i) / (double)size)));
//         const double acos_range_check = std::clamp(beta * cos((M_PI * i) / (double)size), -1.0, 1.0);
//         // const double acos_range_check = beta * cos((M_PI * i) / (double)size);
//         info("acos2: " + STR(acos_range_check));

//         f_win_in[i][0] = (cos((double)size * acos(acos_range_check)))
//                       / (cosh((double)size * acosh(beta)));
//         f_win_in[i][1] = 0.0;
//         info("f_win_in: " + STR(f_win_in[i][0]));
//         info("");
//     }

//     fftwf_execute(p);

//     double normalization_sum = 0.0;
//     for(int i = 0; i < size; i++) {
//         const double norm = sqrt((out[i][0] * out[i][0]) + (out[i][1] * out[i][1]));
//         // const double norm = fabs(out[i]);
//         info(STR(out[i][1]));
//         window[i] = norm;
//         normalization_sum += norm;
//     }

//     // Normalize such that the area under the window function is 1.0
//     for(int i = 0; i < size; i++)
//         window[i] /= normalization_sum;

//     fftwf_destroy_plan(p);
//     fftwf_free(out);
//     fftwf_free(f_win_in);
// }
