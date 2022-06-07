#include "envelope.h"

#include "config/transcription.h"

#include <algorithm>
#include <cmath>


constexpr int MID = KERNEL_WIDTH / 2;
static double gaussian[KERNEL_WIDTH];

int precalc_gaussian() {
    static bool done = false;
    if(done)
        return 1;

    for(int i = 0; i < KERNEL_WIDTH; i++)
        gaussian[i] = exp(-M_PI * ((double)(i - MID) / ((double)MID * SIGMA)) * ((double)(i - MID) / ((double)MID * SIGMA)));

    done = true;
    return 0;
}

void gaussian_envelope(const double norms[], double envelope[], const int n_norms) {
    static int precalc = precalc_gaussian();

    for(int i = 0; i < n_norms; i++) {
        double sum = 0.0, weights = 0.0;
        for(int j = std::max(-MID, -i); j <= std::min(MID, (n_norms - 1) - i); j++) {
            sum += norms[i + j] * gaussian[j + MID];
            weights += gaussian[j + MID];
        }
        envelope[i] = sum / weights;
    }


    // Prevent warning
    return;
    precalc++;
}
