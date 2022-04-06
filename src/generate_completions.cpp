// Included by "parse_cli_args.cpp"

#include "parse_cli_args.h"
#include "error.h"

#include "synth/synth.h"  // Note not synths.h

#include "config/results_file.h"

#include <map>
#include <string>
#include <sstream>
#include <fstream>


constexpr int INDENT_SPACES = 4;  // Cannot be changed, as some indents are hardcoded for code readability
inline std::string indent(const int level) {
    return std::string(level * INDENT_SPACES, ' ');
}


void ArgParser::generate_completions() {
    // Generate the file in a string stream
    std::stringstream ss;

    ss << "# This file is generated using Digistring's completions generator\n"
       << "function _generate_digistring_compl() {\n"
       << "    local cur=${COMP_WORDS[COMP_CWORD]}\n"
       << "\n";

    // Make a list of all flags and count the maximum number of opts for any arg
    std::string all_flags;
    size_t max_number_of_opts = 0;  // Determines the maximum number of opts we have to look back for completions
    for(const auto &[key, value] : flag_to_func) {
        all_flags += key + " ";

        if(value.opt_types.size() > max_number_of_opts)
            max_number_of_opts = value.opt_types.size();
    }

    // Remove trailing space
    if(all_flags.size() > 0)
        all_flags.pop_back();

    // Generate list of all synth types
    std::string all_synths;
    for(const auto &[key, value] : parse_synth_string)
        all_synths += key + " ";

    // Generate rules for when expecting something else than a flag
    for(size_t i = 1; i <= max_number_of_opts; i++) {
        ss << indent(1) << "if (( $COMP_CWORD - " + std::to_string(i) + " >= 1 )); then\n"
           << indent(1) << "    case ${COMP_WORDS[COMP_CWORD - " + std::to_string(i) + "]} in\n";
        for(const auto &[key, value] : flag_to_func) {
            // Don't look more opts back than an arg accepts
            if(value.opt_types.size() < i)
                continue;

            // Generate the rule to match the exception
            ss << indent(3) << key << ")\n";
            switch(value.opt_types[i - 1]) {
                case OptType::dir:
                    ss << indent(4) << "COMPREPLY=($(compgen -A directory -- $cur))\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::file:
                    ss << indent(4) << "COMPREPLY=($(compgen -A file -- $cur))\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::output_file:
                    ss << indent(4) << "if [[ ${#cur} == 0 ]]; then\n"  // Give default output name on empty tab
                       << indent(4) << "    COMPREPLY=(\"" + DEFAULT_OUTPUT_FILENAME + "\")\n"
                       << indent(4) << "else\n"  // Or suggest existing files on non-empty tab
                       << indent(4) << "    COMPREPLY=($(compgen -A file -- $cur))\n"
                       << indent(4) << "fi\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::completions_file:
                    ss << indent(4) << "if [[ ${#cur} == 0 ]]; then\n"  // Give default output name on empty tab
                       << indent(4) << "    COMPREPLY=(\"completions.sh\")\n"
                       << indent(4) << "else\n"  // Or suggest existing files on non-empty tab
                       << indent(4) << "    COMPREPLY=($(compgen -A file -- $cur))\n"
                       << indent(4) << "fi\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::integer:
                    ss << indent(4) << "if [[ ${#cur} == 0 ]]; then\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Please enter an integer${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "elif [[ $cur =~ ^-?[0123456789]+$ ]]; then\n"  // An integer is typed
                       << indent(4) << "    COMPREPLY=(${cur})\n"
                       << indent(4) << "else\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Error: Not an integer${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "fi\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::opt_integer:
                    ss << indent(4) << "if [[ ${#cur} == 0 ]]; then\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Please enter an integer or type - for flag completions${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "elif [[ ${cur[0]} == \"-\" ]]; then\n"  // Flag is started
                       << indent(4) << "    COMPREPLY=($(compgen -W \"" + all_flags + "\" -- $cur))\n"
                       << indent(4) << "elif [[ $cur =~ ^-?[0123456789]+$ ]]; then\n"  // An integer is typed
                       << indent(4) << "    COMPREPLY=(${cur})\n"
                       << indent(4) << "else\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Error: Not an integer or -${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "fi\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::note:
                    ss << indent(4) << "if [[ ${#cur} == 0 ]]; then\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Please enter a note name (e.g. A#4)${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?[0123456789]+$ ]]; then\n"  // Note is fully typed
                       << indent(4) << "    COMPREPLY=(${cur})\n"
                       << indent(4) << "elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?$ ]]; then\n"  // Note is being typed
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Note name example: A#4${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "else\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Error: Not a note name (e.g. A#4)${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "fi\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::opt_note:
                    ss << indent(4) << "if [[ ${#cur} == 0 ]]; then\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Please enter a note name (e.g. A#4) or type - for flag completions${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "elif [[ ${cur[0]} == \"-\" ]]; then\n"  // Flag is started
                       << indent(4) << "    COMPREPLY=($(compgen -W \"" + all_flags + "\" -- $cur))\n"
                       << indent(4) << "elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?[0123456789]+$ ]]; then\n"  // Note is fully typed
                       << indent(4) << "    COMPREPLY=(${cur})\n"
                       << indent(4) << "elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?$ ]]; then\n"  // Note is being typed
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Note name example: A#4${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "else\n"
                       << indent(4) << "    OLD_IFS=\"$IFS\"\n"
                       << indent(4) << "    IFS=$'\\n'\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"Error: Not a note name (e.g. A#4) or -${IFS}...\" -- \"\"))\n"
                       << indent(4) << "    IFS=\"$OLD_IFS\"\n"
                       << indent(4) << "fi\n"
                       << indent(4) << "return 0;;\n";
                    break;

                case OptType::last_arg:
                    ss << indent(4) << "return 0;;\n";
                    break;

                case OptType::opt_synth:
                    ss << indent(4) << "if [[ ${#cur} == 0 ]]; then\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"" + all_synths + " -\" -- $cur))\n"
                       << indent(4) << "elif [[ ${cur[0]} == \"-\" ]]; then\n"  // Flag is started
                       << indent(4) << "    COMPREPLY=($(compgen -W \"" + all_flags + "\" -- $cur))\n"
                       << indent(4) << "else\n"
                       << indent(4) << "    COMPREPLY=($(compgen -W \"" + all_synths + "\" -- $cur))\n"
                       << indent(4) << "fi\n"
                       << indent(4) << "return 0;;\n";
                    break;

                default:
                    error("No completion generation defined for received OptType");
                    exit(EXIT_FAILURE);
            }
        }
        ss << "        esac\n"
           << "    fi\n\n";
    }

    // If code gets here, there was no opt expected for arg, so display all args
    if(all_flags.size() > 0)
        ss << "    COMPREPLY=($(compgen -W \"" + all_flags + "\" -- $cur))\n";

    ss << "    return 0\n"
       << "}\n"
       << "\n"
       << "complete -o filenames -o nosort -F _generate_digistring_compl ./digistring\n"
       << "complete -o filenames -o nosort -F _generate_digistring_compl digistring"
       << std::endl;


    // Output the generated file
    const char *filename;
    if(!fetch_opt(filename)) {
        error("No file provided with generate completions flag");
        exit(EXIT_FAILURE);
    }

    std::fstream out_file(filename, std::ios::out);
    out_file << ss.str();

    exit(EXIT_SUCCESS);
}
