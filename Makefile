CXX := c++
# Warnings are treated as errors so small mistakes do not silently enter the CPU core.
CXXFLAGS := -std=c++17 -Wall -Wextra -Werror -pedantic
PROGRAM ?= examples/add_and_store.asm

.PHONY: all run test clean

all: cpu_demo

# Build the command-line executor and text-program loader.
cpu_demo: main.cpp executor.cpp executor.h cpu.cpp cpu.h
	$(CXX) $(CXXFLAGS) main.cpp executor.cpp cpu.cpp -o cpu_demo

# Tests use the same CPU source as the demo, without another library dependency.
cpu_tests: tests/test_cpu.cpp cpu.cpp cpu.h
	$(CXX) $(CXXFLAGS) tests/test_cpu.cpp cpu.cpp -o cpu_tests

# Parser tests are separate from instruction tests so failures show which layer broke.
executor_tests: tests/test_executor.cpp executor.cpp executor.h cpu.cpp cpu.h
	$(CXX) $(CXXFLAGS) tests/test_executor.cpp executor.cpp cpu.cpp -o executor_tests

run: cpu_demo
	./cpu_demo $(PROGRAM)

test: cpu_tests executor_tests
	./cpu_tests
	./executor_tests

clean:
	rm -f cpu_demo cpu_tests executor_tests
