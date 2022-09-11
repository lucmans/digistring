# This file is generated using Digistring's completions generator
function _generate_digistring_compl() {
    local cur=${COMP_WORDS[COMP_CWORD]}
    local ALL_FLAGS="-f -h -n -o -p -r -s --audio --audio_in --audio_out --experiment --experiments --file --gen-completions --help --midi --output --over --perf --rsc --slow --sync --synth --synths"
    local ALL_EXPERIMENTS="frame_size_limit optimize_xqifft qifft"
    local ALL_SYNTHS="sine sine_amped square"

    if (( $COMP_CWORD - 1 >= 1 )); then
        case ${COMP_WORDS[COMP_CWORD - 1]} in
            -h)
                return 0;;
            -n)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter a note name (e.g. A#4) or type - for flag completions${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ ${cur:0:1} == "-" ]]; then
                    COMPREPLY=($(compgen -W "$ALL_FLAGS" -- $cur))
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
                    COMPREPLY=($(compgen -W "Error: Not a note name (e.g. A#4) or -${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            -o)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("output.json")
                else
                    COMPREPLY=($(compgen -A file -- $cur))
                fi
                return 0;;
            -p)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=($(compgen -W "left right -" -- $cur))
                elif [[ ${cur:0:1} == "-" ]]; then
                    COMPREPLY=($(compgen -W "$ALL_FLAGS" -- $cur))
                else
                    COMPREPLY=($(compgen -W "left right" -- $cur))
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
                    COMPREPLY=($(compgen -W "Please enter a decimal or type - for flag completions${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ ${cur:0:1} == "-" ]]; then
                    COMPREPLY=($(compgen -W "$ALL_FLAGS" -- $cur))
                elif [[ $cur =~ ^[-+]?[0-9]+\.?[0-9]*$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not a decimal or -${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            --audio)
                return 0;;
            --audio_in)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter a recording device name (as printed by Digistring at start-up)${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                else
                    COMPREPLY=(${cur})
                fi
                return 0;;
            --audio_out)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter a playback device name (as printed by Digistring at start-up)${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                else
                    COMPREPLY=(${cur})
                fi
                return 0;;
            --experiment)
                COMPREPLY=($(compgen -W "$ALL_EXPERIMENTS" -- $cur))
                return 0;;
            --experiments)
                return 0;;
            --file)
                COMPREPLY=($(compgen -A file -- $cur))
                return 0;;
            --gen-completions)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("completions.sh")
                else
                    COMPREPLY=($(compgen -A file -- $cur))
                fi
                return 0;;
            --help)
                return 0;;
            --output)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("output.json")
                else
                    COMPREPLY=($(compgen -A file -- $cur))
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
            --perf)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=("perf_statistics.txt")
                else
                    COMPREPLY=($(compgen -A file -- $cur))
                fi
                return 0;;
            --rsc)
                COMPREPLY=($(compgen -A directory -- $cur))
                return 0;;
            --slow)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter a decimal${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ $cur =~ ^[-+]?[0-9]+\.?[0-9]*$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not a decimal..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            --synth)
                if [[ ${#cur} == 0 ]]; then
                    COMPREPLY=($(compgen -W "$ALL_SYNTHS -" -- $cur))
                elif [[ ${cur:0:1} == "-" ]]; then
                    COMPREPLY=($(compgen -W "$ALL_FLAGS" -- $cur))
                else
                    COMPREPLY=($(compgen -W "$ALL_SYNTHS" -- $cur))
                fi
                return 0;;
            --synths)
                return 0;;
        esac
    fi

    if (( $COMP_CWORD - 2 >= 1 )); then
        case ${COMP_WORDS[COMP_CWORD - 2]} in
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
            --gen-completions)
                return 0;;
            --over)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter an integer or type - for flag completions${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ ${cur:0:1} == "-" ]]; then
                    COMPREPLY=($(compgen -W "$ALL_FLAGS" -- $cur))
                elif [[ $cur =~ ^-?[0123456789]+$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not an integer or -${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
            --synth)
                if [[ ${#cur} == 0 ]]; then
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Please enter a decimal or type - for flag completions${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                elif [[ ${cur:0:1} == "-" ]]; then
                    COMPREPLY=($(compgen -W "$ALL_FLAGS" -- $cur))
                elif [[ $cur =~ ^[-+]?[0-9]+\.?[0-9]*$ ]]; then
                    COMPREPLY=(${cur})
                else
                    OLD_IFS="$IFS"
                    IFS=$'\n'
                    COMPREPLY=($(compgen -W "Error: Not a decimal or -${IFS}..." -- ""))
                    IFS="$OLD_IFS"
                fi
                return 0;;
        esac
    fi

    if (( $COMP_CWORD - 3 >= 1 )); then
        case ${COMP_WORDS[COMP_CWORD - 3]} in
            --over)
                COMPREPLY=($(compgen -W "midi_on midi_off" -- $cur))
                return 0;;
        esac
    fi

    if (( $COMP_CWORD - 4 >= 1 )); then
        case ${COMP_WORDS[COMP_CWORD - 4]} in
            --over)
                return 0;;
        esac
    fi

    COMPREPLY=($(compgen -W "$ALL_FLAGS" -- $cur))
    return 0
}

complete -o filenames -o nosort -F _generate_digistring_compl ./digistring
complete -o filenames -o nosort -F _generate_digistring_compl digistring
