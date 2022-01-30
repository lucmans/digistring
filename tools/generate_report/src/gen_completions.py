
BIN_NAME = "./generate_report"


def print_help():
    print("Running 'source completions.sh' adds the completions to the current shell.")
    # print("Copying it to your completions directory (e.g. /usr/share/bash-completion/completions/) installs it.")


def generate(dataset_names):
    with open("completions.sh", "w") as out:
        out.write("""
function _generate_report_comp() {
    local cur=${COMP_WORDS[COMP_CWORD]}

    if [[ ${COMP_CWORD} == 1 ]]; then
        COMPREPLY=(`compgen -W "generate """ + " ".join(dataset_names) + """" -- $cur`)
    elif [[ ${COMP_CWORD} > 1 ]]; then
        if [[ ${COMP_WORDS[1]} == "generate" ]]; then
            return 0
        fi

        local IFS=$'\\n'
        COMPREPLY=(`compgen -A file -- $cur`)
    fi

    return 0
}

complete -o filenames -F _generate_report_comp """ + BIN_NAME + """
""")

    print("Completions written to 'completions.sh'.")
    print_help()
