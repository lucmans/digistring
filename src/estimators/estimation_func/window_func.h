#ifndef DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_WINDOW_FUNC_H
#define DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_WINDOW_FUNC_H


void rectangle_window(double window[], const int size);
void hamming_window(double window[], const int size);
void hann_window(double window[], const int size);
void blackman_window(double window[], const int size);
void nuttall_window(double window[], const int size);
void blackman_nuttall_window(double window[], const int size);
void blackman_harris_window(double window[], const int size);
void flat_top_window(double window[], const int size);
void welch_window(double window[], const int size);
bool dolph_chebyshev_window(double window[], const int size, const double attenuation, const bool cache = false);

void rectangle_window(float window[], const int size);
void hamming_window(float window[], const int size);
void hann_window(float window[], const int size);
void blackman_window(float window[], const int size);
void nuttall_window(float window[], const int size);
void blackman_nuttall_window(float window[], const int size);
void blackman_harris_window(float window[], const int size);
void flat_top_window(float window[], const int size);
void welch_window(float window[], const int size);
bool dolph_chebyshev_window(float window[], const int size, const double attenuation, const bool cache = false);


#endif  // DIGISTRING_ESTIMATORS_ESTIMATION_FUNC_WINDOW_FUNC_H
