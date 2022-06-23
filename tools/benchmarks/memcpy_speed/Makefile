.PHONY: all clean

all:
	@make memcpy_speed --no-print-directory


memcpy_speed: main.cpp
	g++ --std=c++11 -O3 -o $@ $<


clean:
	rm -f memcpy_speed
