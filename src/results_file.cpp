#include "results_file.h"

#include "error.h"

#include "config/results_file.h"

#include <string>
#include <fstream>


ResultsFile::ResultsFile(const std::string &filename) {
    json_file.open(filename, std::fstream::out);
    if(!json_file.is_open()) {
        error("Failed to create/open file '" + filename + "'");
        exit(EXIT_FAILURE);
    }

    cur_indent = 0;
    first_write_block = true;

    json_file << "{" << std::flush;
    cur_indent += 1;
}

ResultsFile::~ResultsFile() {
    json_file << "\n}" << std::endl;
    cur_indent -= 1;

    if(cur_indent != 0)
        warning("JSON output indent was not 1 at the end (was " + STR(cur_indent) + ")\nOutput is possibly ill formatted");
}


void ResultsFile::write_string(const std::string &key, const std::string &value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << key << "\": \"" << value << '"' << std::flush;
    first_write_block = false;
}

void ResultsFile::write_int(const std::string &key, const int value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << key << "\": " << value << std::flush;
    first_write_block = false;
}

void ResultsFile::write_double(const std::string &key, const double value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << key << "\": " << value << std::flush;
    first_write_block = false;
}

void ResultsFile::write_bool(const std::string &key, const bool value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << key << "\": " << (value ? "true" : "false") << std::flush;
    first_write_block = false;
}

void ResultsFile::write_null(const std::string &key) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << key << "\": " << "null" << std::flush;
    first_write_block = false;
}


void ResultsFile::start_array(const std::string &key) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << key << "\": [" << std::flush;

    cur_indent += 1;
    first_write_block = true;
}

void ResultsFile::stop_array() {
    cur_indent -= 1;
    if(first_write_block) {
        json_file << ']';
        first_write_block = false;
        return;
    }

    json_file << "\n"
                  << std::string(cur_indent * INDENT_AMOUNT, ' ') << ']' << std::flush;
}


void ResultsFile::start_dict(const std::string &key) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << key << "\": {" << std::flush;

    cur_indent += 1;
    first_write_block = true;
}

void ResultsFile::stop_dict() {
    cur_indent -= 1;
    if(first_write_block) {
        json_file << '}';
        first_write_block = false;
        return;
    }

    json_file << "\n"
                  << std::string(cur_indent * INDENT_AMOUNT, ' ') << '}' << std::flush;
}



void ResultsFile::write_string(const std::string &value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '"' << value << '"' << std::flush;
    first_write_block = false;
}

void ResultsFile::write_int(const int value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << value << std::flush;
    first_write_block = false;
}

void ResultsFile::write_double(const double value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << value << std::flush;
    first_write_block = false;
}

void ResultsFile::write_bool(const bool value) {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << (value ? "true" : "false") << std::flush;
    first_write_block = false;
}

void ResultsFile::write_null() {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << "null" << std::flush;
    first_write_block = false;
}


void ResultsFile::start_array() {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '[' << std::flush;

    cur_indent += 1;
    first_write_block = true;
}

void ResultsFile::start_dict() {
    if(!first_write_block)
        json_file << ',';
    json_file << '\n';

    json_file << std::string(cur_indent * INDENT_AMOUNT, ' ') << '{' << std::flush;

    cur_indent += 1;
    first_write_block = true;
}
