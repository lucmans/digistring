##
# Makefile for digistring; a program which converts notes played on a guitar to a digital representation in real-time
# @author Luc de Jonckheere
##


# General compiler flags
CXX = g++
CXXFLAGS = -std=c++17 -g
DEPFLAGS = -MT $@ -MMD -MF $(patsubst obj/%.o, dep/%.d, $@)
WARNINGS = -Wall -Wextra #-Wfloat-conversion #-Wconversion #-Warith-conversion #-Wold-style-cast
# FLAGS = -DCOLORED
OPTIMIZATIONS = -O3 #-march=native -mtune=native -mfma -mavx2 -ftree-vectorize -ffast-math
LIBS = -Llib/ -lSDL2 -lSDL2_ttf -lfftw3f -lm
INCL = -Ilib/include/
CORES = 20

SRC_FOLDER = src/ src/estimators/ src/sample_getter/
SRC_FILES = $(patsubst src%/, src%/*.h, $(SRC_FOLDER)) $(patsubst src%/, src%/*.cpp, $(SRC_FOLDER))
BUILD_FOLDERS = $(patsubst src%/, obj%/, $(SRC_FOLDER)) $(patsubst src%/, dep%/, $(SRC_FOLDER))

# Binary name
BIN = digistring
# Objects to compile
OBJ = obj/main.o obj/parse_args.o obj/program.o obj/graphics.o obj/graphics_func.o obj/results_file.o obj/performance.o obj/config.o \
      obj/estimators/estimator.o obj/estimators/highres.o obj/estimators/tuned.o \
      obj/estimators/window_func.o obj/estimators/estimation_func.o obj/spectrum.o obj/note.o \
      obj/sample_getter/sample_getter.o obj/sample_getter/audio_file.o obj/sample_getter/audio_in.o obj/sample_getter/wave_generator.o obj/sample_getter/note_generator.o obj/sample_getter/increment.o

.PHONY: all sanitize force fresh clean outputclean valgrind lines grep debug todo trailing_spaces help


# Makes all folders needed by build process and build with parallel jobs
all: | $(BUILD_FOLDERS)
	make -j $(CORES) $(BIN)

# Don't forget to run make force to remove sanitize
sanitize:
	make force CXXFLAGS="$(CXXFLAGS) -fsanitize=address"

# Remake everything
force:
	make -B all

fresh:
	make clean
	make all

clean:
	rm -rf obj/
	rm -rf dep/
	rm -f $(BIN)
	rm -f *.s
	rm -f vgcore*

outputclean:
	rm -f output*.json


# Binary rule
$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $(WARNINGS) $(OPTIMIZATIONS) -o $@ $^ $(LIBS)


# The different build folders needed
$(BUILD_FOLDERS):
	mkdir -p $@


# Object file rule. Also makes dependency files using $(DEPFLAGS)
obj/%.o: src/%.cpp
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) $(INCL) $(WARNINGS) $(FLAGS) $(OPTIMIZATIONS) -c $< -o $@

# Include the dependencies
include $(wildcard $(patsubst obj/%.o, dep/%.d, $(OBJ)))


# For studying the generated assembly
%.s: %.cpp  %.h
	$(CXX) -S -fverbose-asm -g -O2 $<


valgrind: $(BIN)
	clear
	valgrind --leak-check=full --error-limit=no ./$(BIN)  #--show-reachable=yes

# valgrind: valgrind.supp
# 	clear
# 	valgrind --leak-check=full --error-limit=no --suppressions=valgrind.supp ./$(BIN)  #--show-reachable=yes

# # Generate suppressions for SDL, X11, Intel i965 driver, AMD driver and many other shared libraries
# valgrind.supp:
# 	../gen_val_suppress.py


lines:
	wc -l $(SRC_FILES)


# Usage: make grep pat="pattern"
grep:
# 	@echo $(wordlist 2, $(words $(MAKECMDGOALS)), $(MAKECMDGOALS))
	@grep --color=auto -n "$(pat)" $(SRC_FILES) || echo -e "Pattern not found\n"

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
	@echo \"make outputclean\" removes the default named output files.
	@echo \"make force\" forces all build targets to be rebuild.
	@echo \"make fresh\" runs \"make clean\; make\", which may help with potential building problems after updating.
	@echo \"make sanitize\" builds with -fsanitize=address. Do not forget to run \"make force\" to remove sanitize.
	@echo
	@echo Furthermore, some often used command are added to the makefile:
	@echo \"make lines\" counts the number of lines in all source files.
	@echo \"make grep pat=\'pattern\'\" searches for pattern in all source files.
	@echo \"make debug\" searches for all debug comments in the source files.
	@echo \"make todo\" searches for all todo comments in the source files.
	@echo \"make trailing_spaces\" searches all source files for trailing whitespace characters \(whitespace characters before line end\).
