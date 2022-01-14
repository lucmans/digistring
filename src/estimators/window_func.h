
#ifndef WINDOW_FUNC
#define WINDOW_FUNC


#include "../config.h"


void hamming_window(double window[], const int size);
void hann_window(double window[], const int size);
void blackman_window(double window[], const int size);
void nuttall_window(double window[], const int size);
void blackman_nuttall_window(double window[], const int size);
void blackman_harris_window(double window[], const int size);
void flat_top_window(double window[], const int size);

void hamming_window(float window[], const int size);
void hann_window(float window[], const int size);
void blackman_window(float window[], const int size);
void nuttall_window(float window[], const int size);
void blackman_nuttall_window(float window[], const int size);
void blackman_harris_window(float window[], const int size);
void flat_top_window(float window[], const int size);


#endif  // WINDOW_FUNC
