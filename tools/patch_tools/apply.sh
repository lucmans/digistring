#!/bin/sh

# Current working directory needs to be project root
if [ "$0" != "tools/patch_tools/apply.sh" ]; then
    echo "This program has to be run from the project's root directory"
    exit 1
fi

if [ $# -eq 0 ]; then
    echo "No patch specified"
    exit 1
fi

REVERT=""
if [ "$1" = "revert" ]; then
    REVERT="-R"

    if [ $# -eq 1 ]; then
        echo "No patches specified"
        exit 1
    fi

    shift 1
fi

# for i in $@; do
#     echo "Applying patch '$i'"
#     patch $REVERT -u -p0 -i $i
#     echo ""
# done

echo "Applying patch '$1'"
patch $REVERT -u -p0 -i "$1"
