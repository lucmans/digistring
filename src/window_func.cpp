
#include "window_func.h"


void hamming_window(double window[FRAME_SIZE]) {
    const double N = FRAME_SIZE;
    for(int i = 0; i < FRAME_SIZE; i++) {
        const double x = i * M_PI;

        const double a0 = 25.0 / 46.0;
        window[i] = a0 - ((1.0 - a0) * cos((2.0 * x) / N));
    }
}


void hann_window(double window[FRAME_SIZE]) {
    const double N = FRAME_SIZE;
    for(int i = 0; i < FRAME_SIZE; i++) {
        const double x = i * M_PI;
        window[i] = sin(x / N) * sin(x / N);
    }
}


void blackman_window(double window[FRAME_SIZE]) {
    const double N = FRAME_SIZE;
    for(int i = 0; i < FRAME_SIZE; i++) {
        const double x = i * M_PI;

        const double a0 = 7938.0 / 18608.0;
        const double a1 = 9240.0 / 18608.0;
        const double a2 = 1430.0 / 18608.0;
        window[i] = a0 - (a1 * cos((2.0 * x) / N)) + (a2 * cos((4.0 * x) / N));
    }
}


void nuttall_window(double window[FRAME_SIZE]) {
    const double N = FRAME_SIZE;
    for(int i = 0; i < FRAME_SIZE; i++) {
        const double x = i * M_PI;

        const double a0 = 0.355768;
        const double a1 = 0.487396;
        const double a2 = 0.144232;
        const double a3 = 0.012604;
        window[i] = a0 - (a1 * cos((2.0 * x) / N)) + (a2 * cos((4.0 * x) / N)) - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_nuttall_window(double window[FRAME_SIZE]) {
    const double N = FRAME_SIZE;
    for(int i = 0; i < FRAME_SIZE; i++) {
        const double x = i * M_PI;

        const double a0 = 0.3635819;
        const double a1 = 0.4891775;
        const double a2 = 0.1365995;
        const double a3 = 0.0106411;
        window[i] = a0 - (a1 * cos((2.0 * x) / N)) + (a2 * cos((4.0 * x) / N)) - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_harris_window(double window[FRAME_SIZE]) {
    const double N = FRAME_SIZE;
    for(int i = 0; i < FRAME_SIZE; i++) {
        const double x = i * M_PI;

        const double a0 = 0.35875;
        const double a1 = 0.48829;
        const double a2 = 0.14128;
        const double a3 = 0.01168;
        window[i] = a0 - (a1 * cos((2.0 * x) / N)) + (a2 * cos((4.0 * x) / N)) - (a3 * cos((6.0 * x) / N));
    }
}


void flat_top_window(double window[FRAME_SIZE]) {
    const double N = FRAME_SIZE;
    for(int i = 0; i < FRAME_SIZE; i++) {
        const double x = i * M_PI;

        const double a0 = 0.21557895;
        const double a1 = 0.41663158;
        const double a2 = 0.277263158;
        const double a3 = 0.083578947;
        const double a4 = 0.006947368;
        window[i] = a0 - (a1 * cos((2.0 * x) / N)) + (a2 * cos((4.0 * x) / N)) - (a3 * cos((6.0 * x) / N)) + (a4 * cos((8.0 * x) / N));
    }
}
