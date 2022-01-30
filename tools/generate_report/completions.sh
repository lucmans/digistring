
function _generate_report_comp() {
    local cur=${COMP_WORDS[COMP_CWORD]}

    if [[ ${COMP_CWORD} == 1 ]]; then
        COMPREPLY=(`compgen -W "generate fraunhofer MDB-stem-synth" -- $cur`)
    elif [[ ${COMP_CWORD} > 1 ]]; then
        if [[ ${COMP_WORDS[1]} == "generate" ]]; then
            return 0
        fi

        local IFS=$'\n'
        COMPREPLY=(`compgen -A file -- $cur`)
    fi

    return 0
}

complete -o filenames -F _generate_report_comp ./generate_report
