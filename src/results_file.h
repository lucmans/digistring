#ifndef DIGISTRING_RESULTS_FILE_H
#define DIGISTRING_RESULTS_FILE_H


#include <string>
#include <fstream>


// This class simply overwrites the given file; be warned
// Furthermore, the caller is responsible for generating correct JSON
// Possible mistakes include:
//   - Providing keys in arrays
//   - Not ending a dictionary or array (in the right order if nested)
//   - Not escaping ASCII " and accepted UTF-8 variants in string keys and values
//   - Providing empty key
class ResultsFile {
    public:
        ResultsFile(const std::string &filename);
        ~ResultsFile();

        void write_string(const std::string &key, const std::string &value);
        void write_int(const std::string &key, const int value);
        void write_double(const std::string &key, const double value);
        void write_bool(const std::string &key, const bool value);
        void write_null(const std::string &key);

        void start_array(const std::string &key);
        void stop_array();

        void start_dict(const std::string &key);
        void stop_dict();

        // For when in an array
        void write_string(const std::string &value);
        void write_int(const int value);
        void write_double(const double value);
        void write_bool(const bool value);
        void write_null();

        void start_array();
        void start_dict();


    private:
        std::fstream output_stream;

        int cur_indent;
        bool first_write_block;  // Prevents writing ",\n" on first writes of new block
};


#endif  // DIGISTRING_RESULTS_FILE_H
