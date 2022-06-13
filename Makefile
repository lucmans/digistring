##
# Makefile for digistring; a program which converts notes played on a guitar to a digital representation in real-time
# @author Luc de Jonckheere
##

# Binary name
BIN = digistring

# General compiler flags
# c++20 is required for std::source_location (error.h)
CXX = g++
CXXFLAGS = -std=c++20 -g -fopenmp
DEPFLAGS = -MT $@ -MMD -MF $(patsubst obj/%.o, dep/%.d, $@)
WARNINGS = -Wall -Wextra -Wshadow -pedantic -Wstrict-aliasing -Wfloat-equal #-Wfloat-conversion #-Wconversion #-Warith-conversion #-Wold-style-cast
OPTIMIZATIONS = -O3 #-march=native -mtune=native -mfma -mavx2 -ftree-vectorize -ffast-math
LIBS = -Llib/ -lSDL2 -lSDL2_ttf -lfftw3f -lm
INCL = -Isrc/ -Ilib/include/
CORES = 20

# Compile time config; run "make force" after change
# -DNO_COLORS: Don't output escape sequences used for coloring in terminal
# -DINFO_SOURCE_LOC: Print the source location of info messages
# -DMSG_PRINT_TIME: Print time since program start in logging messages
COMPILE_CONFIG =

SRC_FOLDERS = $(patsubst %, %/, $(shell find src -type d -print))
# SRC_FILES = $(patsubst src%/, src%/*.h, $(SRC_FOLDERS)) $(patsubst src%/, src%/*.cpp, $(SRC_FOLDERS))
SRC_FILES = $(wildcard $(patsubst src%/, src%/*.h, $(SRC_FOLDERS))) $(wildcard $(patsubst src%/, src%/*.cpp, $(SRC_FOLDERS)))
BUILD_FOLDERS = $(patsubst src%/, obj%/, $(SRC_FOLDERS)) $(patsubst src%/, dep%/, $(SRC_FOLDERS))

# Object files to create (source files to compile)
# Create an .o file for every .cpp file
ALL_OBJ = $(patsubst src/%.cpp, obj/%.o, $(wildcard $(patsubst src%/, src%/*.cpp, $(SRC_FOLDERS))))
# .o files which should not be created (.cpp which should not be compiled)
FILTER_OBJ = obj/generate_completions.o
OBJ = $(filter-out $(FILTER_OBJ), $(ALL_OBJ))

.PHONY: all sanitize force fresh ubuntu2104 ubuntu2004lts clean cacheclean outputclean valgrind check_patches lines grep grepl debug todo trailing_spaces help


# Makes all folders needed by build process and build with parallel jobs
all:
	make -j $(CORES) $(BIN)

# Don't forget to run make force to remove sanitize
sanitize:
	make force CXXFLAGS="$(CXXFLAGS) -fsanitize=address"

# Remake everything
force:
	make -B all

fresh:
	make clean --no-print-directory
	make all

ubuntu2104:
	make clean --no-print-directory
	tools/patch_tools/check.sh patches/c++17.patch
	tools/patch_tools/apply.sh patches/c++17.patch

ubuntu2004lts:
	make clean --no-print-directory
	tools/patch_tools/check.sh
	tools/patch_tools/apply.sh patches/c++17.patch
	tools/patch_tools/apply.sh patches/glibc-2_31.patch

clean:
	rm -rf obj/
	rm -rf dep/
	rm -f $(BIN)
	rm -f *.s
	rm -f vgcore*

cacheclean:
	rm -rf cache/

outputclean:
	rm -f output*.json


# Binary rule
$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(OPTIMIZATIONS) -o $@ $^ $(LIBS)


# The different build folders needed
$(BUILD_FOLDERS):
	mkdir -p $@


# Object file rule. Also makes dependency files using $(DEPFLAGS)
obj/%.o: src/%.cpp | $(BUILD_FOLDERS)
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCL) $(WARNINGS) $(COMPILE_CONFIG) $(OPTIMIZATIONS) -c $< -o $@

# Include the dependencies
include $(wildcard $(patsubst obj/%.o, dep/%.d, $(OBJ)))


# For studying the generated assembly
src/%.s: src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCL) $(WARNINGS) $(COMPILE_CONFIG) $(OPTIMIZATIONS) -S $< -fverbose-asm


valgrind: all
	clear
	valgrind --leak-check=full --error-limit=no ./$(BIN)  #--show-reachable=yes

# valgrind: valgrind.supp
# 	clear
# 	valgrind --leak-check=full --error-limit=no --suppressions=valgrind.supp ./$(BIN)  #--show-reachable=yes

# # Generate suppressions for SDL, X11, Intel i965 driver, AMD driver and many other shared libraries
# valgrind.supp:
# 	../gen_val_suppress.py


check_patches:
	@tools/patch_tools/check.sh


lines:
	@echo -e "\033[1mPython tools\033[0m"
	wc -l tools/generate_report/src/*.py
	@echo
	wc -l tools/dolph_chebyshev_window/src/*.py
	@echo
	wc -l tools/performance_plot/src/*.py
	@echo
	@echo -e "\033[1mDelayed playback tool\033[0m"
	make -C tools/delayed_playback/ --no-print-directory lines
	@echo -e "\n\033[1mDigistring (C++) code\033[0m"
	wc -l $(SRC_FILES)


# Usage: make grep pat="pattern"
grep:
	@grep --color=auto -n "$(pat)" $(SRC_FILES) || echo -e "Pattern not found\n"

grepl:
	@grep --color=auto -F -n '$(pat)' $(SRC_FILES) || echo -e "Pattern not found\n"

debug:
	@grep --color=auto -n DEBUG $(SRC_FILES) || echo -e "No debug code left!\n"

todo:
	@grep --color=auto -n TODO $(SRC_FILES) || echo -e "Nothing left to do!\n"

# Note that when copy pasting the grep command, the extra $ has to be removed
trailing_spaces:
	@grep --color=auto -n -r '[[:blank:]]$$' $(SRC_FILES) Makefile || echo -e "No trailing spaces in source!\n"


help:
	@echo The default build target is \"all\", which builds the binary \"$(BIN)\".
	@echo \"make clean\" removes all built files.
	@echo \"make cacheclean\" removes the Digistring cache directory.
	@echo \"make outputclean\" removes the default named output files.
	@echo \"make force\" forces all build targets to be rebuild.
	@echo \"make fresh\" runs \"make clean\; make\", which may help with potential building problems after updating.
	@echo \"make ubuntu2104\" applies all patches necessary for Ubuntu 21.04 support.
	@echo \"make ubuntu2004lts\" applies all patches necessary for Ubuntu 20.04 LTS support.
	@echo \"make sanitize\" builds with -fsanitize=address. Do not forget to run \"make force\" to remove sanitize.
	@echo
	@echo Furthermore, some often used command are added to the makefile:
	@echo \"make check_patches\" checks if all patches are still applicable \(does not check if code still compiles/runs\).
	@echo \"make lines\" counts the number of lines in all source files.
	@echo \"make grep pat=\'pattern\'\" searches for pattern in all source files.
	@echo \"make grepl pat=\'pattern\'\" is the same as \"make grep\" but with fixed string patterns \(no regex\).
	@echo \"make debug\" searches for all debug comments in the source files.
	@echo \"make todo\" searches for all todo comments in the source files.
	@echo \"make trailing_spaces\" searches all source files for trailing whitespace characters \(whitespace characters before line end\).
