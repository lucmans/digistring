#!/bin/sh

output_diff() {
    ORIG_FILE=$1
    shift 1
    PATCHED_FILE=$1
    shift 1

    echo "Generating diff between '$ORIG_FILE' and '$PATCHED_FILE'"
    diff -u $ORIG_FILE $PATCHED_FILE --label $ORIG_FILE --label $ORIG_FILE >> $PATCH_NAME
    RET=$?
    if [ $RET -eq 0 ]; then
        echo "No difference between '$ORIG_FILE' and '$PATCHED_FILE'"
    elif [ $RET -eq 2 ]; then
        echo "Error while diff-ing '$ORIG_FILE' and '$PATCHED_FILE'"
        rm $PATCH_NAME
        exit 1
    fi
}

# Current working directory needs to be project root for correct file paths in patch file
if [ $0 != "tools/patch_tools/create.sh" ]; then
    echo "This program has to be run from the project's root directory"
    exit 1
fi

# Need a patch_name, original file and patched file at minimum
if [[ $# < 3 ]]; then
    echo "Not enough arguments specified"
    echo "Usage: tools/patch_tools/create.sh patch_name original_file1 patched_file1 [original_file2 patched_file2 ...]"
    exit 1
fi

# Files always come in pairs (old-new file)
# Check for even number of arguments, as first argument is patch name
if [ $(($# % 2)) -eq 0 ]; then
    echo "Error: odd number of files specified"
    exit 1
fi

# Check if output file already exists
PATCH_NAME=$1
if [ -e $PATCH_NAME ]; then
    echo "Patch already exists"
    exit 1
fi

# Create the patch
touch $PATCH_NAME
shift 1
TOTAL=$#

# First outside the loop, so extra newlines can be inserted between patches
output_diff $@
shift 2
for (( i=2; i<$TOTAL; i+=2 )); do
    echo -e "\n" >> $PATCH_NAME
    output_diff $@
    shift 2
done

echo -e "\nSuccessfully generated patch $PATCH_NAME"
