#!/bin/sh

# Current working directory needs to be project root
if [ $0 != "tools/patch_tools/check.sh" ]; then
    echo "This program has to be run from the project's root directory"
    exit 1
fi

PROBLEMS=0
for i in patches/*.patch; do
    echo "Checking '$i'"
    patch -u -p0 -i $i --dry-run || PROBLEMS=1
    echo ""
done

if [[ PROBLEMS -eq 0 ]]; then
    echo "All patches can be applied (didn't check if code still compiles/runs correctly)"
else
    echo "One or more patches could not be applied!"
fi
