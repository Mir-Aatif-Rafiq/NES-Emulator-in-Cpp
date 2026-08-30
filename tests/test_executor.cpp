#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../executor.h"

bool expect(bool condition, const std::string& test_name) {
	if (!condition) {
		std::cerr << "[failed] " << test_name << '\n';
		return false;
	}
	std::cout << "[passed] " << test_name << '\n';
	return true;
}

class ExecutorMachine {
public:
	CPU6502 cpu;
	RAM ram;
	Bus bus;
	Executor executor;

	ExecutorMachine() : bus(cpu, ram), executor(cpu, bus) {
	}
};

bool test_parse_load_and_run() {
	ExecutorMachine machine;
	std::istringstream source(
		".ORG $8000\n"
		"LDA #$05\n"
		"CLC\n"
		"ADC #$03\n"
		"STA $0200\n"
		".END\n");

	machine.executor.loadSource(source, "addition test");
	const Executor::Result result = machine.executor.run();
	return expect(result.reason == Executor::StopReason::EndOfProgram &&
		result.instructions == 4 && machine.bus.bus_read(0x0200) == 0x08,
		"parse, load and execute an assembly stream");
}

bool test_load_program_from_file() {
	ExecutorMachine machine;
	machine.executor.loadFile("examples/countdown.asm");
	const Executor::Result result = machine.executor.run();
	return expect(result.reason == Executor::StopReason::EndOfProgram &&
		machine.cpu.X == 0x00 && machine.bus.bus_read(0x0200) == 0x00,
		"read, assemble and run a program from a text file");
}

bool test_labels_and_relative_branch() {
	ExecutorMachine machine;
	std::istringstream source(
		"LDX #3\n"
		"loop: DEX\n"
		"BNE loop\n"
		"STX $0200\n");

	machine.executor.loadSource(source, "label test");
	const Executor::Result result = machine.executor.run();
	return expect(result.reason == Executor::StopReason::EndOfProgram &&
		result.instructions == 8 && machine.cpu.X == 0x00 &&
		machine.bus.bus_read(0x0200) == 0x00,
		"resolve a label and encode a signed branch offset");
}

bool test_addressing_mode_selection() {
	ExecutorMachine machine;
	std::istringstream source(
		"LDA $10\n"
		"LDA $1234\n"
		"LDA $10,X\n"
		"LDA ($20),Y\n");

	machine.executor.loadSource(source, "addressing test");
	return expect(machine.bus.bus_read(0x8000) == 0xA5 &&
		machine.bus.bus_read(0x8002) == 0xAD &&
		machine.bus.bus_read(0x8005) == 0xB5 &&
		machine.bus.bus_read(0x8007) == 0xB1,
		"select opcodes from operand addressing syntax");
}

bool test_instruction_limit() {
	ExecutorMachine machine;
	std::istringstream source(
		"loop: JMP loop\n"
		".END\n");

	machine.executor.loadSource(source, "loop test");
	const Executor::Result result = machine.executor.run(10);
	return expect(result.reason == Executor::StopReason::InstructionLimit &&
		result.instructions == 10,
		"stop an accidental infinite loop at the instruction limit");
}

bool test_invalid_program_reports_error() {
	ExecutorMachine machine;
	std::istringstream source("LDA missing_label\n");
	try {
		machine.executor.loadSource(source, "bad program");
	} catch (const std::runtime_error& error) {
		return expect(std::string(error.what()).find("undefined label") !=
			std::string::npos, "report an undefined label");
	}
	return expect(false, "report an undefined label");
}

int main() {
	bool passed = true;
	passed &= test_parse_load_and_run();
	passed &= test_load_program_from_file();
	passed &= test_labels_and_relative_branch();
	passed &= test_addressing_mode_selection();
	passed &= test_instruction_limit();
	passed &= test_invalid_program_reports_error();

	if (!passed) {
		return 1;
	}
	std::cout << "All executor tests passed.\n";
	return 0;
}
