#include "estimator.h"

#include "estimator_graphics/spectrum.h"
#include "error.h"

#include "config/graphics.h"

#include <fftw3.h>

#include <map>
#include <string>
#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE


Estimator::Estimator() {
    estimator_graphics = nullptr;
}

Estimator::~Estimator() {

}


const EstimatorGraphics *Estimator::get_estimator_graphics() {
    if constexpr(HEADLESS) {
        error("This call should never occur with headless mode");
        exit(EXIT_FAILURE);
    }

    return estimator_graphics;
}


void Estimator::next_plot_type() {
    estimator_graphics->next_plot();
}
