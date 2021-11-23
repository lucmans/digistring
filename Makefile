##
# Makefile for digistring; a program which converts notes played on a guitar to a digital representation in real-time
# @author Luc de Jonckheere
##


# General compiler flags
CXX = g++
CXXFLAGS = -std=c++17 -g  #-fsanitize=address
DEPFLAGS = -MT $@ -MMD -MF $(patsubst obj/%.o, dep/%.d, $@)
WARNINGS = -Wall -Wextra #-Wfloat-conversion #-Wconversion -Warith-conversion
# FLAGS = -DCOLORED
OPTIMIZATIONS = -O3 #-march=native -mtune=native -mfma -mavx2 -ftree-vectorize -ffast-math
LIBS = -Llib/ -lSDL2 -lSDL2_ttf -lfftw3f -lm
INCL = -Ilib/include/
CORES = 20

SRC_FILES = src/*.h src/*.cpp src/estimators/*.h src/estimators/*.cpp
BUILD_FOLDERS = obj/ obj/estimators/ dep/ dep/estimators/

BIN = digistring
OBJ = obj/main.o obj/program.o obj/graphics.o obj/graphics_func.o obj/sample_getter.o obj/performance.o obj/config.o \
      obj/estimators/estimator.o obj/estimators/highres.o obj/estimators/tuned.o \
      obj/estimators/window_func.o obj/estimators/estimation_func.o obj/spectrum.o obj/note.o

.PHONY: all force fresh clean valgrind lines debug todo trailing_spaces


# Makes all folders needed by build process and build with parallel jobs
all: | $(BUILD_FOLDERS)
	make -j $(CORES) $(BIN)

# Remake everything
force:
	make -B all

fresh:
	make clean
	make all


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


debug:
	grep -n DEBUG $(SRC_FILES) || echo -e "No debug code left!\n"

todo:
	grep -n TODO $(SRC_FILES) || echo -e "Nothing left to do!\n"


# Note that when copy pasting the grep command, the extra $ has to be removed
trailing_spaces:
	@grep -n -r '[[:blank:]]$$' $(SRC_FILES) Makefile || echo -e "No trailing spaces in source!\n"


clean:
	rm -rf obj/
	rm -rf dep/
	rm -f $(BIN)
	rm -f *.s
	rm -f vgcore*
