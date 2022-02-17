
#include "parse_cli_args.h"
#include "error.h"

#include "config/results_file.h"

#include <map>
#include <string>
#include <sstream>
#include <fstream>


void ArgParser::generate_completions() {
    // Generate the file in a string stream
    std::stringstream ss;

    ss << "# This file is generated using Digistring's completions generator\n"
       << "function _generate_digistring_compl() {\n"
       << "    local cur=${COMP_WORDS[COMP_CWORD]}\n"
       << "\n";

    // Make a list of all flags and count the maximum number of opts for any arg
    std::string all_flags;
    size_t max_number_of_opts = 0;
    for(const auto &[key, value] : flag_to_func) {
        all_flags += " " + key;

        if(value.opt_types.size() > max_number_of_opts)
            max_number_of_opts = value.opt_types.size();
    }

    // Remove leading space
    if(all_flags.size() > 0)
        all_flags.erase(0, 1);

    // Generate rules for when expecting something else than a flag
    for(size_t i = 1; i <= max_number_of_opts; i++) {
        ss << "    if (( $COMP_CWORD - 1 >= 1 )); then\n"
           << "        case ${COMP_WORDS[COMP_CWORD - " + std::to_string(i) + "]} in\n";
        for(const auto &[key, value] : flag_to_func) {
            // Don't look more opts back than an arg accepts
            if(value.opt_types.size() < i)
                continue;

            // Generate the rule to match the exception
            ss << "            " << key << ")\n";
            switch(value.opt_types[i - 1]) {
                case OptType::dir:
                    ss << "                COMPREPLY=(`compgen -A directory -- $cur`)\n"
                       << "                return 0;;\n";
                    break;

                case OptType::file:
                    ss << "                COMPREPLY=(`compgen -A file -- $cur`)\n"
                       << "                return 0;;\n";
                    break;

                case OptType::output_file:
                    ss << "                if [[ ${#cur} == 0 ]]; then\n"  // Give default output name on empty tab
                       << "                    COMPREPLY=(\"" + DEFAULT_OUTPUT_FILENAME + "\")\n"
                       << "                else\n"  // Or suggest existing files on non-empty tab
                       << "                    COMPREPLY=(`compgen -A file -- $cur`)\n"
                       << "                fi\n"
                       << "                return 0;;\n";
                    break;

                case OptType::completions_file:
                    ss << "                if [[ ${#cur} == 0 ]]; then\n"  // Give default output name on empty tab
                       << "                    COMPREPLY=(\"completions.sh\")\n"
                       << "                else\n"  // Or suggest existing files on non-empty tab
                       << "                    COMPREPLY=(`compgen -A file -- $cur`)\n"
                       << "                fi\n"
                       << "                return 0;;\n";
                    break;

                case OptType::integer:
                    ss << "                if [[ ${#cur} == 0 ]]; then\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Please enter an integer${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                elif [[ $cur =~ ^-?[0123456789]+$ ]]; then\n"  // An integer is typed
                       << "                    COMPREPLY=(${cur})\n"
                       << "                else\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Error: Not an integer${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                fi\n"
                       << "                return 0;;\n";
                    break;

                case OptType::opt_integer:
                    ss << "                if [[ ${#cur} == 0 ]]; then\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Please enter an integer or type \'-\' for flag completions${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                elif [[ ${cur[0]} == \"-\" ]]; then\n"  // Flag is started
                       << "                    COMPREPLY=(`compgen -W \"" + all_flags + "\" -- $cur`)\n"
                       << "                elif [[ $cur =~ ^-?[0123456789]+$ ]]; then\n"  // An integer is typed
                       << "                    COMPREPLY=(${cur})\n"
                       << "                else\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Error: Not an integer or '-'${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                fi\n"
                       << "                return 0;;\n";
                    break;

                case OptType::note:
                    ss << "                if [[ ${#cur} == 0 ]]; then\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Please enter a note name (e.g. A#4)${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?[0123456789]+$ ]]; then\n"  // Note is fully typed
                       << "                    COMPREPLY=(${cur})\n"
                       << "                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?$ ]]; then\n"  // Note is being typed
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Note name example: A#4${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                else\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Error: Not a note name (e.g. A#4)${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                fi\n"
                       << "                return 0;;\n";
                    break;

                case OptType::opt_note:
                    ss << "                if [[ ${#cur} == 0 ]]; then\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Please enter a note name (e.g. A#4) or type \'-\' for flag completions${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                elif [[ ${cur[0]} == \"-\" ]]; then\n"  // Flag is started
                       << "                    COMPREPLY=(`compgen -W \"" + all_flags + "\" -- $cur`)\n"
                       << "                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?[0123456789]+$ ]]; then\n"  // Note is fully typed
                       << "                    COMPREPLY=(${cur})\n"
                       << "                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?$ ]]; then\n"  // Note is being typed
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Note name example: A#4${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                else\n"
                       << "                    OLD_IFS=\"$IFS\"\n"
                       << "                    IFS=$'\\n'\n"
                       << "                    COMPREPLY=($(compgen -W \"Error: Not a note name (e.g. A#4) or '-'${IFS}...\" -- \"\"))\n"
                       << "                    IFS=\"$OLD_IFS\"\n"
                       << "                fi\n"
                       << "                return 0;;\n";
                    break;

                case OptType::last_arg:
                    ss << "                return 0;;\n";
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
        ss << "    COMPREPLY=(`compgen -W \"" + all_flags + "\" -- $cur`)\n";

    ss << "    return 0\n"
       << "}\n"
       << "\n"
       << "complete -o filenames -o nosort -F _generate_digistring_compl ./digistring"
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
