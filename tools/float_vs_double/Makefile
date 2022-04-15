.PHONY: all clean

all:
	@make --no-print-directory float_vs_double
	@make --no-print-directory float_vs_double_O3
	@make --no-print-directory float_vs_double_fast


float_vs_double: main.cpp
	g++ --std=c++11 -o $@ $<

float_vs_double_O3: main.cpp
	g++ --std=c++11 -o $@ -O3 $<

float_vs_double_fast: main.cpp
	g++ --std=c++11 -o $@ -O3 -march=native -mtune=native -mfma -mavx2 -ftree-vectorize -ffast-math $<


clean:
	rm -f float_vs_double float_vs_double_O3 float_vs_double_fast
