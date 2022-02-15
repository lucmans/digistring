
#include "window_func.h"

#include <cmath>

// For Dolph Chebyshev window (calling Python code and reading the output file)
#include <string>
#include <filesystem>
#include <fstream>
#include <chrono>
#include <cstring>  // std::strerror()
#include <cstdlib>  // EXIT_SUCCESS, EXIT_FAILURE
#include <unistd.h>  // execvp
#include <sys/wait.h>  // waitpid()

#include "error.h"

#include "config/transcription.h"
#include "config/cli_args.h"


void rectangle_window(double window[], const int size) {
    for(int i = 0; i < size; i++)
        window[i] = 1.0;
}


void hamming_window(double window[], const int size) {
    const double a0 = 25.0 / 46.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;

        window[i] = a0 - ((1.0 - a0) * cos((2.0 * x) / N));
    }
}


void hann_window(double window[], const int size) {
    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = sin(x / N) * sin(x / N);
    }
}


void blackman_window(double window[], const int size) {
    const double a0 = 7938.0 / 18608.0;
    const double a1 = 9240.0 / 18608.0;
    const double a2 = 1430.0 / 18608.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N));
    }
}


void nuttall_window(double window[], const int size) {
    const double a0 = 0.355768;
    const double a1 = 0.487396;
    const double a2 = 0.144232;
    const double a3 = 0.012604;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_nuttall_window(double window[], const int size) {
    const double a0 = 0.3635819;
    const double a1 = 0.4891775;
    const double a2 = 0.1365995;
    const double a3 = 0.0106411;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_harris_window(double window[], const int size) {
    const double a0 = 0.35875;
    const double a1 = 0.48829;
    const double a2 = 0.14128;
    const double a3 = 0.01168;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void flat_top_window(double window[], const int size) {
    const double a0 = 0.21557895;
    const double a1 = 0.41663158;
    const double a2 = 0.277263158;
    const double a3 = 0.083578947;
    const double a4 = 0.006947368;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N))
                       + (a4 * cos((8.0 * x) / N));
    }
}


void welch_window(double window[], const int size) {
    const double N = size;
    const double hN = (double)size / 2.0;
    for(int i = 0; i < N; i++) {
        const double t = (i - hN) / hN;
        window[i] = 1 - (t * t);
    }
}


// There two helper functions are defined at the bottom
std::string generate_tmp_filename(const std::string &o_filename);
bool run_python_script(const std::string &tmp_file_path, const std::string &size, const std::string &attenuation);

bool dolph_chebyshev_window(double window[], const int size, const double attenuation) {
    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    info("Generating Dolph Chebyshev window using Python, this may take some time...");

    const std::string tmp_file_path = cli_args.rsc_dir + "/" + generate_tmp_filename(TMP_DOLPH_WIN_FILENAME);

    // Bubble the return value up; error is printed in the function
    if(!run_python_script(tmp_file_path, std::to_string(size), std::to_string(attenuation)))
        return false;

    // Read the output file of the Python script
    std::fstream output_file(tmp_file_path);
    for(int i = 0; i < size; i++)
        output_file >> window[i];

    // Remove the output file
    if(!std::filesystem::remove(tmp_file_path)) {
        warning("Failed to delete '" + tmp_file_path + "' after computing the Dolph Chebyshev window\nPlease remove manually");
        return false;
    }

    // Output performance measurement
    const double duration = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_time).count();
    info("Dolph Chebyshev window created (" + STR(duration) + " ms)");

    return true;
}



void rectangle_window(float window[], const int size) {
    for(int i = 0; i < size; i++)
        window[i] = 1.0;
}


void hamming_window(float window[], const int size) {
    const double a0 = 25.0 / 46.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;

        window[i] = a0 - ((1.0 - a0) * cos((2.0 * x) / N));
    }
}


void hann_window(float window[], const int size) {
    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = sin(x / N) * sin(x / N);
    }
}


void blackman_window(float window[], const int size) {
    const double a0 = 7938.0 / 18608.0;
    const double a1 = 9240.0 / 18608.0;
    const double a2 = 1430.0 / 18608.0;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N));
    }
}


void nuttall_window(float window[], const int size) {
    const double a0 = 0.355768;
    const double a1 = 0.487396;
    const double a2 = 0.144232;
    const double a3 = 0.012604;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_nuttall_window(float window[], const int size) {
    const double a0 = 0.3635819;
    const double a1 = 0.4891775;
    const double a2 = 0.1365995;
    const double a3 = 0.0106411;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void blackman_harris_window(float window[], const int size) {
    const double a0 = 0.35875;
    const double a1 = 0.48829;
    const double a2 = 0.14128;
    const double a3 = 0.01168;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N));
    }
}


void flat_top_window(float window[], const int size) {
    const double a0 = 0.21557895;
    const double a1 = 0.41663158;
    const double a2 = 0.277263158;
    const double a3 = 0.083578947;
    const double a4 = 0.006947368;

    const double N = size;
    for(int i = 0; i < N; i++) {
        const double x = i * M_PI;
        window[i] = a0 - (a1 * cos((2.0 * x) / N))
                       + (a2 * cos((4.0 * x) / N))
                       - (a3 * cos((6.0 * x) / N))
                       + (a4 * cos((8.0 * x) / N));
    }
}


void welch_window(float window[], const int size) {
    const double N = size;
    const double hN = (double)size / 2.0;
    for(int i = 0; i < N; i++) {
        const double t = (i - hN) / hN;
        window[i] = 1 - (t * t);
    }
}


std::string generate_tmp_filename(const std::string &o_filename) {
    // Separate basename and extension
    std::string o_basename, o_extension;
    const size_t pos = o_filename.find_last_of('.');
    if(pos == std::string::npos || pos == 0)
        o_basename = o_filename;
    else {  // pos > 0
        o_basename = o_filename.substr(0, pos);
        o_extension = o_filename.substr(pos);
    }

    // Try to generate a new unique filename inserting a number between name and extension
    std::string g_filename = o_filename;  // Generated filename
    for(int i = 2; std::filesystem::exists(cli_args.rsc_dir + "/" + g_filename); i++)
        g_filename = o_basename + '_' + std::to_string(i) + o_extension;

    return g_filename;
}

bool run_python_script(const std::string &tmp_file_path, const std::string &size, const std::string &attenuation) {
    const pid_t pid = fork();
    if(pid == -1) {
        warning("Failed to fork process\nOS error: " + std::string(std::strerror(errno)));
        return false;
    }
    else if(pid == 0) {
        // Child process
        const std::string python_cmd = "python3";
        const std::string python_script = cli_args.rsc_dir + "/../tools/dolph_chebyshev_window/dolph_chebyshev_window";
        const char *child_argv[] = {python_cmd.c_str(),
                                    python_script.c_str(),
                                    tmp_file_path.c_str(),
                                    size.c_str(),
                                    attenuation.c_str(),
                                    NULL};
        const int ret = execvp(python_cmd.c_str(), const_cast<char *const *>(child_argv));
        if(ret == -1) {
            warning("Failed to run Python in child process (using '" + python_cmd + "')\nOS error: " + std::string(std::strerror(errno)));
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
    else {
        // Parent process
        int status;
        const pid_t ret_pid = waitpid(pid, &status, 0);
        if(ret_pid == -1) {
            warning("waitpid() returned -1\nOS error: " + std::string(std::strerror(errno)));  // TODO: Better warning
            return false;
        }

        if(ret_pid != pid) {
            warning("PID mismatch");  // TODO: Better warning
            return false;
        }

        if(!WIFEXITED(status)) {
            warning("waitpid() status argument hint abnormal child process exit");
            return false;
        }

        if(WEXITSTATUS(status) == 127) {
            warning("Failed execv (127 return code)");
            return false;
        }

        if(WEXITSTATUS(status) != 0) {
            warning("Python program exited with non-zero exit code (Python error should be printed above)");
            return false;
        }
    }

    return true;
}

bool dolph_chebyshev_window(float window[], const int size, const double attenuation) {
    const std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
    info("Generating Dolph Chebyshev window using Python, this may take some time...");

    const std::string tmp_file_path = cli_args.rsc_dir + "/" + generate_tmp_filename(TMP_DOLPH_WIN_FILENAME);

    // Bubble the return value up; error is printed in the function
    if(!run_python_script(tmp_file_path, std::to_string(size), std::to_string(attenuation)))
        return false;

    // Read the output file of the Python script
    std::fstream output_file(tmp_file_path);
    for(int i = 0; i < size; i++)
        output_file >> window[i];

    // Remove the output file
    if(!std::filesystem::remove(tmp_file_path)) {
        warning("Failed to delete '" + tmp_file_path + "' after computing the Dolph Chebyshev window\nPlease remove manually");
        return false;
    }

    // Output performance measurement
    const double duration = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start_time).count();
    info("Dolph Chebyshev window created (" + STR(duration) + " ms)");

    return true;
}

// #include <fftw3.h>
// #include <algorithm>
// #include "../error.h"
// void dolph_chebyshev_window(float window[], const int size) {
//     // The Dolph-Chebyshev is defined in the frequency domain
//     fftwf_complex *f_win_in = (fftwf_complex*)fftwf_malloc(size * sizeof(fftwf_complex));
//     fftwf_complex *out = (fftwf_complex*)fftwf_malloc(size * sizeof(fftwf_complex));
//     fftwf_plan p = fftwf_plan_dft_1d(size, f_win_in, out, FFTW_BACKWARD, FFTW_ESTIMATE);

//     // Alpha
//     const double dB = -60.0;
//     const double alpha = dB / -20.0;
//     const double beta = cosh((1.0 / (double)size) * acosh(pow(10.0, alpha)));

//     info("dB: " + STR(dB));
//     info("alpha: " + STR(alpha));
//     info("beta: " + STR(beta));
//     info("");

//     for(int i = 0; i < size; i++) {
//         info("acos: " + STR(beta * cos((M_PI * i) / (double)size)));
//         const double acos_range_check = std::clamp(beta * cos((M_PI * i) / (double)size), -1.0, 1.0);
//         // const double acos_range_check = beta * cos((M_PI * i) / (double)size);
//         info("acos2: " + STR(acos_range_check));

//         f_win_in[i][0] = (cos((double)size * acos(acos_range_check)))
//                       / (cosh((double)size * acosh(beta)));
//         f_win_in[i][1] = 0.0;
//         info("f_win_in: " + STR(f_win_in[i][0]));
//         info("");
//     }

//     fftwf_execute(p);

//     double normalization_sum = 0.0;
//     for(int i = 0; i < size; i++) {
//         const double norm = sqrt((out[i][0] * out[i][0]) + (out[i][1] * out[i][1]));
//         // const double norm = fabs(out[i]);
//         info(STR(out[i][1]));
//         window[i] = norm;
//         normalization_sum += norm;
//     }

//     // Normalize such that the area under the window function is 1.0
//     for(int i = 0; i < size; i++)
//         window[i] /= normalization_sum;

//     fftwf_destroy_plan(p);
//     fftwf_free(out);
//     fftwf_free(f_win_in);
// }
