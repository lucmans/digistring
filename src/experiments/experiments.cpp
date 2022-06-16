#include "experiments.h"

#include "error.h"
#include "quit.h"

#include "config/audio.h"
#include "config/transcription.h"

#include "qifft.h"


void qifft_errors() {
    if constexpr(ZERO_PAD_FACTOR > 0.0) {
        info("Assuming frame size of " + STR(FRAME_SIZE) + " samples with " + STR(FRAME_SIZE_PADDED - FRAME_SIZE) + " samples zero-padding\n"
        "  - Frame time: " + STR(((double)FRAME_SIZE / (double)SAMPLE_RATE) * 1000.0) + " ms\n"
        "  - Fourier bin size: " + STR((double)SAMPLE_RATE / (double)FRAME_SIZE) + " Hz\n"
        "  - Frame size with zero padding: " + STR(FRAME_SIZE_PADDED) + " samples\n"
        "  - Interpolated bin size: " + STR((double)SAMPLE_RATE / (double)FRAME_SIZE_PADDED) + " Hz\n");
    }
    else {
        info("Assuming frame size of " + STR(FRAME_SIZE) + " samples\n"
        "  - Frame time: " + STR(((double)FRAME_SIZE / (double)SAMPLE_RATE) * 1000.0) + " ms\n"
        "  - Fourier bin size: " + STR((double)SAMPLE_RATE / (double)FRAME_SIZE) + " Hz\n");
    }

    QIFFT qifft = QIFFT(FRAME_SIZE, FRAME_SIZE_PADDED - FRAME_SIZE);
    ErrorMeasures error = qifft.no_qifft();
    std::cout << "Nearest bin: "       << OPTI_MEASURE_STR << " " << get_opti_measure(error) << OPTI_MEASURE_UNIT_STR << std::endl;
    if(poll_quit()) return;

    error = qifft.mqifft();
    std::cout << "MQIFFT: "            << OPTI_MEASURE_STR << " " << get_opti_measure(error) << OPTI_MEASURE_UNIT_STR << std::endl;
    if(poll_quit()) return;

    error = qifft.lqifft();
    std::cout << "LQIFFT with ln: "    << OPTI_MEASURE_STR << " " << get_opti_measure(error) << OPTI_MEASURE_UNIT_STR << std::endl;
    if(poll_quit()) return;

    error = qifft.lqifft2();
    std::cout << "LQIFFT with log2: "  << OPTI_MEASURE_STR << " " << get_opti_measure(error) << OPTI_MEASURE_UNIT_STR << std::endl;
    if(poll_quit()) return;

    error = qifft.lqifft10();
    std::cout << "LQIFFT with log10: " << OPTI_MEASURE_STR << " " << get_opti_measure(error) << OPTI_MEASURE_UNIT_STR << std::endl;
    if(poll_quit()) return;

    error = qifft.dbqifft();
    std::cout << "dB-QIFFT: "          << OPTI_MEASURE_STR << " " << get_opti_measure(error) << OPTI_MEASURE_UNIT_STR << std::endl;
}

void optimize_qxifft() {
    info("Using HighRes estimator defaults...");
    iteratively_optimize_qxifft(FRAME_SIZE, FRAME_SIZE_PADDED - FRAME_SIZE);
}
