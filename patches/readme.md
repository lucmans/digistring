On older systems, building may fail as Digistring uses the newest features from its dependencies. Not all of these new features are necessary and some features may still be used through an older interface. To accommodate older systems, we provide the patches given in this directory.  
In order to use a patch, run the following command from the root of the project (where `<patch_name>` is replaced by the desired patch file):  
`patch -u -p0 -i patches/<patch_name>`  
To revert a patch, run the following command:  
`patch -R -u -p0 -i patches/<patch_name>`  
The command can be used from any directory by using the `-d <path_to_project_root>` flag.


# C++ 17 support (`c++17.patch`)
C++ 20 is currently only used for printing debug information and designated initializers. These C++ 20 features can easily be replaced. The rest of the project does have a hard C++ 17 requirement.  
To use this patch, run the following command from the root of the project:  
`patch -u -p0 -i patches/c++17.patch`


# glibc 2.31 support (`glibc-2_31.patch`)
In order to get the name of a signal number, the function `sigabbrev_np()` is used. This function first appeared in glibc version 2.32. We can simply omit printing the signal name.  
To use this patch, run the following command from the root of the project:  
`patch -u -p0 -i patches/glibc-2_31.patch`
