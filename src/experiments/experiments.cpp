#include "experiments.h"

#include "error.h"
#include "quit.h"

#include "config/transcription.h"

#include "qifft.h"


void qifft_errors() {
    warning("Assuming frame size of " + STR(FRAME_SIZE) + " samples with " + STR(FRAME_SIZE_PADDED - FRAME_SIZE) + " samples zero-padding"
        ". You are responsible the optimization pitch estimation process matches the one the parameter is optimized for.");

    QIFFT qifft = QIFFT(FRAME_SIZE, FRAME_SIZE_PADDED - FRAME_SIZE);
    double error = qifft.no_qifft();
    std::cout << "Nearest bin: "       << error << " mean squared error" << std::endl;
    if(poll_quit()) return;

    error = qifft.mqifft();
    std::cout << "MQIFFT: "            << error << " mean squared error" << std::endl;
    if(poll_quit()) return;

    error = qifft.lqifft();
    std::cout << "LQIFFT with ln: "    << error << " mean squared error" << std::endl;
    if(poll_quit()) return;

    error = qifft.lqifft2();
    std::cout << "LQIFFT with log2: "  << error << " mean squared error" << std::endl;
    if(poll_quit()) return;

    error = qifft.lqifft10();
    std::cout << "LQIFFT with log10: " << error << " mean squared error" << std::endl;
    if(poll_quit()) return;

    error = qifft.dbqifft();
    std::cout << "dB-QIFFT: "          << error << " mean squared error" << std::endl;
}

void optimize_qxifft() {
    info("Using HighRes estimator defaults...");
    iteratively_optimize_qxifft(FRAME_SIZE, FRAME_SIZE_PADDED - FRAME_SIZE);
}
