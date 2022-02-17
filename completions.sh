# This file is generated using Digistring's completions generator
function _generate_digistring_compl() {
    local cur=${COMP_WORDS[COMP_CWORD]}

    if (( $COMP_CWORD - 1 >= 1 )); then
        case ${COMP_WORDS[COMP_CWORD - 1]} in
            --file)
                COMPREPLY=(`compgen -A file -- $cur`)
                return 0;;
            --generate-completions)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("completions.sh")
                else
                    COMPREPLY=(`compgen -A file -- $cur`)
                fi
                return 0;;
            --output)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("output.json")
                else
                    COMPREPLY=(`compgen -A file -- $cur`)
                fi
                return 0;;
            --over)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter a note name (e.g. A#4)${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?[0123456789]+$ ]]; then
                    COMPREPLY=(${cur})
                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?$ ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Note name example: A#4${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not a note name (e.g. A#4)${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            --rsc)
                COMPREPLY=(`compgen -A directory -- $cur`)
                return 0;;
            -g)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("completions.sh")
                else
                    COMPREPLY=(`compgen -A file -- $cur`)
                fi
                return 0;;
            -n)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter a note name (e.g. A#4) or type '-' for flag completions${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ ${cur[0]} == "-" ]]; then
                    COMPREPLY=(`compgen -W "--file --generate-completions --help --output --over --perf --rsc -f -g -h -n -o -p -r -s" -- $cur`)
                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?[0123456789]+$ ]]; then
                    COMPREPLY=(${cur})
                elif [[ $cur =~ ^[ABCDEFGabcdefg][#db]?$ ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Note name example: A#4${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not a note name (e.g. A#4) or '-'${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            -o)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("output.json")
                else
                    COMPREPLY=(`compgen -A file -- $cur`)
                fi
                return 0;;
            -r)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter an integer${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ $cur =~ ^-?[0123456789]+$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not an integer${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            -s)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter an integer or type '-' for flag completions${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ ${cur[0]} == "-" ]]; then
                    COMPREPLY=(`compgen -W "--file --generate-completions --help --output --over --perf --rsc -f -g -h -n -o -p -r -s" -- $cur`)
                elif [[ $cur =~ ^-?[0123456789]+$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not an integer or '-'${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
        esac
    fi

    if (( $COMP_CWORD - 1 >= 1 )); then
        case ${COMP_WORDS[COMP_CWORD - 2]} in
            --generate-completions)
                return 0;;
            --over)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter an integer or type '-' for flag completions${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ ${cur[0]} == "-" ]]; then
                    COMPREPLY=(`compgen -W "--file --generate-completions --help --output --over --perf --rsc -f -g -h -n -o -p -r -s" -- $cur`)
                elif [[ $cur =~ ^-?[0123456789]+$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not an integer or '-'${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            -g)
                return 0;;
            -r)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter an integer${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ $cur =~ ^-?[0123456789]+$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not an integer${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
        esac
    fi

    if (( $COMP_CWORD - 1 >= 1 )); then
        case ${COMP_WORDS[COMP_CWORD - 3]} in
            --over)
                return 0;;
        esac
    fi

    COMPREPLY=(`compgen -W "--file --generate-completions --help --output --over --perf --rsc -f -g -h -n -o -p -r -s" -- $cur`)
    return 0
}

complete -o filenames -o nosort -F _generate_digistring_compl ./digistring
