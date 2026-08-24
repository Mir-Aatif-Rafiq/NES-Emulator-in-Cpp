CXX := c++
# Warnings are treated as errors so small mistakes do not silently enter the CPU core.
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic

.PHONY: all run test clean

all: cpu_demo

# Build the small machine-code example from main.cpp.
cpu_demo: main.cpp cpu.cpp cpu.h
	$(CXX) $(CXXFLAGS) main.cpp cpu.cpp -o cpu_demo

# Tests use the same CPU source as the demo, without another library dependency.
cpu_tests: tests/test_cpu.cpp cpu.cpp cpu.h
	$(CXX) $(CXXFLAGS) tests/test_cpu.cpp cpu.cpp -o cpu_tests

run: cpu_demo
	./cpu_demo

test: cpu_tests
	./cpu_tests

clean:
	rm -f cpu_demo cpu_tests
