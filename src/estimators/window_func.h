
#ifndef WINDOW_FUNC
#define WINDOW_FUNC


#include "../config.h"


void hamming_window(double window[FRAME_SIZE]);
void hann_window(double window[FRAME_SIZE]);
void blackman_window(double window[FRAME_SIZE]);
void nuttall_window(double window[FRAME_SIZE]);
void blackman_nuttall_window(double window[FRAME_SIZE]);
void blackman_harris_window(double window[FRAME_SIZE]);
void flat_top_window(double window[FRAME_SIZE]);


#endif  // WINDOW_FUNC
