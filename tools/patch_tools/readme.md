To help with creating and applying patches, we have created three different tools.  
These scripts are POSIX/Bourne shell compliant and rely on core utils such as echo, rm and touch. Furthermore, diff and patch are used for creating and applying patches.  
All of these tools should only be run from the project's root directory.


# apply.sh
Applies patch to source code. Can optionally revert a patch.  
Usage: `tools/patch_tools/apply.sh [revert] <patch_file>`

This script simply calls `patch -u -p0 -i <patch_file>` (also adds `-R` when revert was passed). In case of script failures, try running that command manually from the project's root directory.


# create.sh
Creates patch files which can be used through `apply.sh`.  
Usage: `tools/patch_tools/create.sh patch_name original_file1 patched_file1 [original_file2 patched_file2 ...]`


# check.sh
Checks if patches can still be applied to the source files. It does not check if the code still compiles or runs correctly.  
Usage: `tools/patch_tools/check.sh <patch_file>`
