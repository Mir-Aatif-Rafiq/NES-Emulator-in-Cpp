#include <exception>
#include <iomanip>
#include <iostream>

#include "cpu.h"
#include "executor.h"

int main(int argument_count, char* arguments[]) {
	CPU6502 cpu;
	RAM ram;
	Bus bus(cpu, ram);
	Executor executor(cpu, bus);
	const std::string program_file = argument_count > 1
		? arguments[1] : "examples/add_and_store.asm";

	try {
		// The executor now owns the complete host-side flow: parse the text,
		// assemble it, load RAM, set the reset vector and run the CPU.
		executor.loadFile(program_file);
		const Executor::Result result = executor.run();

		std::cout << "Loaded: " << program_file << '\n'
			<< "Stopped: " << stopReasonName(result.reason) << '\n'
			<< "Instructions: " << std::dec << result.instructions << '\n'
			<< "Cycles (including reset): " << result.cycles << '\n'
			<< "PC: $" << std::hex << std::uppercase << std::setw(4)
			<< std::setfill('0') << result.final_pc << '\n'
			<< "A: $" << std::setw(2) << static_cast<int>(cpu.A)
			<< "  X: $" << std::setw(2) << static_cast<int>(cpu.X)
			<< "  Y: $" << std::setw(2) << static_cast<int>(cpu.Y) << '\n';

		return result.reason == Executor::StopReason::EndOfProgram ? 0 : 2;
	} catch (const std::exception& error) {
		std::cerr << "Executor error: " << error.what() << '\n';
		return 1;
	}
}
