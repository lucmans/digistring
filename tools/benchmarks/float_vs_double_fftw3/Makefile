.PHONY: all clean

all:
	@make float --no-print-directory
	@make double --no-print-directory


float: main_float.cpp
	g++ --std=c++11 -o $@ -O2 $< -lfftw3f -lm

double: main_double.cpp
	g++ --std=c++11 -o $@ -O2 $< -lfftw3 -lm


clean:
	rm -f float double
