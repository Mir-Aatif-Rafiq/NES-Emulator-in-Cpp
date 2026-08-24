CXX := c++
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic

.PHONY: all run test clean

all: cpu_demo

cpu_demo: main.cpp cpu.cpp cpu.h
	$(CXX) $(CXXFLAGS) main.cpp cpu.cpp -o cpu_demo

cpu_tests: tests/test_cpu.cpp cpu.cpp cpu.h
	$(CXX) $(CXXFLAGS) tests/test_cpu.cpp cpu.cpp -o cpu_tests

run: cpu_demo
	./cpu_demo

test: cpu_tests
	./cpu_tests

clean:
	rm -f cpu_demo cpu_tests
