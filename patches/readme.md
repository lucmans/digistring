# Patch usage
On older systems, building may fail as Digistring may use the newest features from its dependencies. Not all of these new features are necessary and some features may still be used through an older interface. To accommodate older systems, we provide the patches given in this directory.  
In order to use a patch, run the following command from the root of the project (where `<patch_name>` is replaced by the desired patch file):  
`tools/patch_tools/apply.sh patches/<patch_name>`  
To revert a patch, run the following command:  
`tools/patch_tools/apply.sh revert patches/<patch_name>`  


# Patch creation
To create a new patch `<patch_name>`, which changes the original source code `<original_file>` (as cloned from the repository) to the new patched file `<patched_file>`, run the following command from the root of the project:  
`tools/patch_tools/create.sh patches/<patch_name> <original_file> <patched_file>`  
Optionally, more `<original_file> <patched_file>` pairs may be given to the script to create a patch effecting multiple files.


# Patches
There are currently two patches. These are provided to support Ubuntu 20.04 LTS, but may help for other older system too.

## C++17 support (`c++17.patch`)
C++20 is currently only used for printing debug information and designated initializers. These C++20 features can easily be replaced. The rest of the project does have a hard C++17 requirement.  
Affected files: `Makefile` `src/error.h` `src/graphics.cpp`

## glibc 2.31 support (`glibc-2_31.patch`)
In order to get the name of a signal number, the function `sigabbrev_np()` is used. This function first appeared in glibc version 2.32. We can simply omit printing the signal name.
Affected files: `src/quit.cpp`
