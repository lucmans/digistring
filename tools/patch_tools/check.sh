#!/bin/sh

# Current working directory needs to be project root
if [ "$0" != "tools/patch_tools/check.sh" ]; then
    echo "This program has to be run from the project's root directory"
    exit 1
fi

# Check singular patch
if [ $# -eq 1 ]; then
    if [ ! -e "$1" ]; then
        echo "Given patch does not exist"
        exit 1
    fi

    patch -u -p0 -i "$1" --dry-run
    RET=$?
    if [ $RET -ne 0 ]; then
        echo "Failed to apply patch!"
        exit 1
    fi

    echo "Patch can be applied"
    exit 0
elif [ $# -gt 1 ]; then
    echo "Error: Can only pass one patch at the time"
    exit 1
fi

# Check all patches
PROBLEMS=0
for i in patches/*.patch; do
    echo "Checking '$i'"
    patch -u -p0 -i "$i" --dry-run || PROBLEMS=1
    echo ""
done

if [ $PROBLEMS -eq 0 ]; then
    echo "All patches can be applied (didn't check if code still compiles/runs correctly)"
else
    echo "One or more patches could not be applied!"
fi
