#include "qifft.h"

#include "estimators/estimation_func/window_func.h"
#include "estimators/estimation_func/norms.h"
#include "estimators/estimation_func/interpolate_peaks.h"
#include "error.h"
#include "quit.h"

#include "config/audio.h"

#include <omp.h>
#include <fftw3.h>

#include <algorithm>
#include <vector>
#include <utility>
#include <functional>
#include <sstream>


const int REPS_PER_FREQ = 8;


// Needed because there interpolation funcs are overloaded version
typedef double(*interpolation_func_t)(double, double, double, double&);


std::vector<double> generate_test_freqs() {
    std::vector<double> freqs;
    for(int i = 0; i < 1200; i++)
        freqs.push_back(110.0 * exp2((double)i / 1200.0));
        // freqs.push_back(110.0 * (((double)i / 1200.0) + 1.0));

    return freqs;
}


std::vector<std::pair<double, double>> make_range(const double range_min, const double range_max, const double step_size) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"
    std::vector<std::pair<double, double>> exps;
    for(double e = range_min; e < range_max; e += step_size) {
        if(e == 0.0)
            warning("Skipping invalid exponent (value of 0.0)...");
        else
            exps.push_back({e, 0.0});
    }
    #pragma GCC diagnostic pop

    return exps;
}

double iteratively_optimize_qxifft(const int frame_size, const int padding_size/*, const window_func*/) {
    warning("Assuming frame size of " + STR(frame_size) + " samples with " + STR(padding_size) + " samples zero-padding"
            ". You are responsible the optimization pitch estimation process matches the one the parameter is optimized for.");
    const int n_cores = omp_get_num_procs();
    const int min_steps = 8;

    double min_range = -10.0;
    double max_range = 10.0;
    double step_size = (max_range - min_range) / std::max(n_cores, min_steps);

    std::stringstream ss;
    ss << "Looking between " << min_range << " and " << max_range << " with a step size of " << step_size;
    info(ss.str());

    std::vector<std::pair<double, double>> exps = make_range(min_range, max_range, step_size);
    if(exps.size() == 0) {
        warning("Range only contains 0.0");
        return 0.0;
    }

    std::vector<QIFFT *> transformers;
    for(int i = 0; i < n_cores; i++)
        transformers.push_back(new QIFFT(frame_size, padding_size));

    int min_idx = -1;
    while(!poll_quit()) {
        const int n_tasks = exps.size();
        int last_task = 0;
        #pragma omp parallel num_threads(n_cores)
        {
            while(true) {
                int thread_num = omp_get_thread_num();
                int task;
                #pragma omp critical
                {
                    task = last_task;
                    last_task++;
                }
                if(task >= n_tasks)
                    break;

                exps[task].second = transformers[thread_num]->xqifft_exp_error(exps[task].first);
            }
        }

        min_idx = 0;
        std::cout << "exp " << exps[0].first << " -> mean squared error " << exps[0].second /*<< " Hz"*/ << std::endl;
        for(int i = 1; i < n_tasks; i++) {
            std::cout << "exp " << exps[i].first << " -> mean squared error " << exps[i].second /*<< " Hz"*/ << std::endl;
            if(exps[i].second < exps[min_idx].second)
                min_idx = i;
        }
        std::cout << std::endl;

        if(poll_quit())
            break;

        if(min_idx == 0) {
            std::cout << "Minimum value is at the start of range; moving search range down" << std::endl;
            min_range -= step_size * (double)(n_tasks - 2);
            max_range -= step_size * (double)(n_tasks - 2);
        }
        else if(min_idx == n_tasks - 1) {
            std::cout << "Minimum value is at the end of range; moving search range up" << std::endl;
            min_range += step_size * (double)(n_tasks - 2);
            max_range += step_size * (double)(n_tasks - 2);
        }
        else {
            std::cout << "Zooming in between " << exps[min_idx - 1].first << " and " << exps[min_idx + 1].first << "..." << std::endl;
            min_range = exps[min_idx - 1].first;
            max_range = exps[min_idx + 1].first;
            step_size = (max_range - min_range) / std::max(n_cores, min_steps);
        }

        exps.clear();
        exps = make_range(min_range, max_range, step_size);
    }

    if(min_idx != -1) {
        std::stringstream().swap(ss);  // Create new (empty) stream
        ss << "Best found exponent is " << exps[min_idx].first << " with a mean squared error of " << exps[min_idx].second /*<< " Hz"*/;
        info(ss.str());
    }

    for(int i = 0; i < n_cores; i++)
        delete transformers[i];

    return exps[min_idx].first;
}


QIFFT::QIFFT(const int _frame_size, const int _padding_size/*, const window_func*/)
        : frame_size(_frame_size), padding_size(_padding_size), in_size(_frame_size + _padding_size) {
    in = (float*)fftwf_malloc(in_size * sizeof(float));
    if(in == NULL) {
        error("Failed to allocate input buffer");
        exit(EXIT_FAILURE);
    }
    // Zero zero-padded part of buffer
    std::fill_n(in + frame_size, padding_size, 0.0);  // memset() might be faster, but assumes IEEE 754 floats/doubles

    out = (fftwf_complex*)fftwf_malloc(((in_size / 2) + 1) * sizeof(fftwf_complex));
    if(out == NULL) {
        error("Failed to allocate Fourier output buffer");
        exit(EXIT_FAILURE);
    }

    // TODO: Exhaustive planner
    p = fftwf_plan_dft_r2c_1d(in_size, in, out, FFTW_ESTIMATE);
    if(p == NULL) {
        error("Failed to create FFTW3 plan");
        exit(EXIT_FAILURE);
    }

    try {
        window_func = new float[frame_size];
    }
    catch(const std::bad_alloc &e) {
        error("Failed to allocate window function buffer");
        exit(EXIT_FAILURE);
    }
    dolph_chebyshev_window(window_func, frame_size, 50.0, true);

    try {
        norms = new double[in_size];
    }
    catch(const std::bad_alloc &e) {
        error("Failed to allocate norms buffer");
        exit(EXIT_FAILURE);
    }
}

QIFFT::~QIFFT() {
    delete[] norms;
    delete[] window_func;
    fftwf_destroy_plan(p);
    fftwf_free(out);
    fftwf_free(in);
}


double QIFFT::qifft_error(const std::function<double(double, double, double, double&)> interpolation_func) {
    const std::vector<double> freqs = generate_test_freqs();

    double last_phase;
    double total_error = 0.0;
    double total_squared_error = 0.0;
    double max_error = 0.0;
    for(const double freq : freqs) {
        last_phase = 0.0;
        for(int r = 0; r < REPS_PER_FREQ; r++) {
            const double phase_offset = last_phase * ((double)SAMPLE_RATE / freq);
            for(int i = 0; i < in_size; i++)
                in[i] = sinf((2.0 * M_PI * ((double)i + phase_offset) * freq) / (double)SAMPLE_RATE);
            last_phase = fmod(last_phase + (freq / ((double)SAMPLE_RATE / (double)in_size)), 1.0);

            for(int i = 0; i < frame_size; i++)
                in[i] *= window_func[i];

            fftwf_execute(p);

            calc_norms(out, norms, (in_size / 2) + 1);

            int peak_idx = 1;
            for(int i = 2; i < (in_size / 2); i++)
                if(norms[i] > norms[peak_idx])
                    peak_idx = i;

            double amp;
            const double offset = interpolation_func(norms[peak_idx], norms[peak_idx - 1], norms[peak_idx + 1], amp);

            const double detected_freq = (peak_idx + offset) * ((double)SAMPLE_RATE / (double)in_size);
            // const double cent_error = 1200.0 * log2(detected_freq / freq);
            const double hz_error = detected_freq - freq;

            const double squared_error = hz_error * hz_error;
            total_squared_error += squared_error;

            const double abs_error = abs(hz_error);
            total_error += abs_error;
            max_error = std::max(max_error, abs_error);
        }
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
    const double mean_error = total_error / (REPS_PER_FREQ * (double)freqs.size());
    const double mean_squared_error = total_squared_error / (REPS_PER_FREQ * (double)freqs.size());
    #pragma GCC diagnostic pop

    return mean_squared_error;
}

double QIFFT::no_qifft() {
    return qifft_error([](const double peak, const double l, const double r, double &amp){amp = peak; return 0.0; amp = l+r;});
}

// Static casts are needed as interpolation funcs are overloaded
double QIFFT::mqifft() {
    return qifft_error(static_cast<interpolation_func_t>(&interpolate_max));
}

double QIFFT::lqifft() {
    return qifft_error(static_cast<interpolation_func_t>(&interpolate_max_log));
}

double QIFFT::lqifft2() {
    return qifft_error(static_cast<interpolation_func_t>(&interpolate_max_log2));
}

double QIFFT::lqifft10() {
    return qifft_error(static_cast<interpolation_func_t>(&interpolate_max_log10));
}

double QIFFT::dbqifft() {
    return qifft_error(static_cast<interpolation_func_t>(&interpolate_max_db));
}


double QIFFT::xqifft_exp_error(const double exp) {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wfloat-equal"
    if(exp == 0.0) {
        warning("Exponent can't be 0.0");
        return -1.0;
    }
    #pragma GCC diagnostic pop

    const std::vector<double> freqs = generate_test_freqs();

    double last_phase;
    double total_error = 0.0;
    double total_squared_error = 0.0;
    double max_error = 0.0;
    for(const double freq : freqs) {
        last_phase = 0.0;
        for(int r = 0; r < REPS_PER_FREQ; r++) {
            const double phase_offset = last_phase * ((double)SAMPLE_RATE / freq);
            for(int i = 0; i < in_size; i++)
                in[i] = sinf((2.0 * M_PI * ((double)i + phase_offset) * freq) / (double)SAMPLE_RATE);
            last_phase = fmod(last_phase + (freq / ((double)SAMPLE_RATE / (double)in_size)), 1.0);

            for(int i = 0; i < frame_size; i++)
                in[i] *= window_func[i];

            fftwf_execute(p);

            calc_norms(out, norms, (in_size / 2) + 1);

            int peak_idx = 1;
            for(int i = 2; i < (in_size / 2); i++)
                if(norms[i] > norms[peak_idx])
                    peak_idx = i;

            double amp;
            const double offset = interpolate_max_exp(norms[peak_idx], norms[peak_idx - 1], norms[peak_idx + 1], exp, amp);

            const double detected_freq = (peak_idx + offset) * ((double)SAMPLE_RATE / (double)in_size);
            // const double cent_error = 1200.0 * log2(detected_freq / freq);
            const double hz_error = detected_freq - freq;

            const double squared_error = hz_error * hz_error;
            total_squared_error += squared_error;

            const double abs_error = abs(hz_error);
            total_error += abs_error;
            max_error = std::max(max_error, abs_error);
        }
    }

    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-variable"
    const double mean_error = total_error / (REPS_PER_FREQ * (double)freqs.size());
    const double mean_squared_error = total_squared_error / (REPS_PER_FREQ * (double)freqs.size());
    #pragma GCC diagnostic pop

    return mean_squared_error;
}


double QIFFT::xqifft_exp_test_range(const double check_min, const double check_max, const double check_step, const bool print /*= false*/) {
    double out_error;
    return xqifft_exp_test_range(check_min, check_max, check_step, out_error, print);
}

double QIFFT::xqifft_exp_test_range(const double check_min, const double check_max, const double check_step, double &out_error, const bool print /*= false*/) {
    if(check_min >= check_max) {
        warning("Upper bound is equal or lower than lower bound");
        return 0.0;
    }
    if(check_step <= 0.0) {
        warning("Step size should be > 0.0");
        return 0.0;
    }

    std::vector<std::pair<double, double>> exps = make_range(check_min, check_max, check_step);
    if(exps.size() == 0) {
        warning("Range only contains 0.0");
        return 0.0;
    }

    for(auto &[exp, mean_squared_error] : exps) {
        mean_squared_error = xqifft_exp_error(exp);

        if(print)
            std::cout << "exp " << exp << " -> mean squared error " << mean_squared_error << std::endl;
    }

    // Find best exponent
    int min_idx = 0;
    const int n_exps = exps.size();
    for(int i = 1; i < n_exps; i++)
        if(exps[i].second < exps[min_idx].second)
            min_idx = i;

    out_error = exps[min_idx].second;
    return exps[min_idx].first;
}
