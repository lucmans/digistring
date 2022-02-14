
#ifndef WINDOW_FUNC
#define WINDOW_FUNC


#include "../config.h"


void rectangle_window(double window[], const int size);
void hamming_window(double window[], const int size);
void hann_window(double window[], const int size);
void blackman_window(double window[], const int size);
void nuttall_window(double window[], const int size);
void blackman_nuttall_window(double window[], const int size);
void blackman_harris_window(double window[], const int size);
void flat_top_window(double window[], const int size);
void welch_window(double window[], const int size);
bool dolph_chebyshev_window(double window[], const int size, const double attenuation);

void rectangle_window(float window[], const int size);
void hamming_window(float window[], const int size);
void hann_window(float window[], const int size);
void blackman_window(float window[], const int size);
void nuttall_window(float window[], const int size);
void blackman_nuttall_window(float window[], const int size);
void blackman_harris_window(float window[], const int size);
void flat_top_window(float window[], const int size);
void welch_window(float window[], const int size);
bool dolph_chebyshev_window(float window[], const int size, const double attenuation);


#endif  // WINDOW_FUNC
