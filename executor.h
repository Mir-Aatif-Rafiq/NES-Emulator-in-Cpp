#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cpu.h"

/*
* The CPU only understands bytes. Executor is the host-side part which turns a
* small 6502 assembly text file into those bytes, loads them through the bus and
* repeatedly asks the CPU to execute one instruction.
*/
class Executor {
public:
	enum class StopReason {
		EndOfProgram,
		InstructionLimit
	};

	struct Result {
		StopReason reason;
		std::size_t instructions;
		u_int64_t cycles;
		u_int16_t final_pc;
	};

	Executor(CPU6502& cpu, Bus& bus);

	// loadFile is the normal entry point. loadSource also makes the parser easy
	// to test without creating a temporary file for every test case.
	void loadFile(const std::string& file_name);
	void loadSource(std::istream& source, const std::string& source_name = "input");
	Result run(std::size_t maximum_instructions = 100000);

	u_int16_t startAddress() const;
	u_int16_t endAddress() const;

private:
	enum class AddressMode {
		IMP, IMM, ZP0, ZPX, ZPY, REL, ABS, ABX, ABY, IND, IZX, IZY
	};

	struct SourceLine {
		std::size_t number = 0;
		std::string operation;
		std::string operand;
		u_int16_t address = 0;
		AddressMode mode = AddressMode::IMP;
		std::size_t size = 0;
		bool byte_directive = false;
	};

	CPU6502& cpu;
	Bus& bus;
	bool program_loaded = false;
	u_int16_t start_address = 0x8000;
	u_int16_t end_address = 0x8000;

	std::unordered_map<std::string, u_int16_t> labels;
	std::vector<SourceLine> lines;
	std::vector<std::pair<u_int16_t, u_int8_t>> image;

	AddressMode selectMode(const std::string& operation,
		const std::string& operand, std::size_t line_number) const;
	u_int8_t findOpcode(const std::string& operation, AddressMode mode,
		std::size_t line_number) const;
	u_int16_t resolveValue(const std::string& token,
		std::size_t line_number) const;
	bool hasOpcode(const std::string& operation, AddressMode mode) const;
	void firstPass(std::istream& source, const std::string& source_name);
	void secondPass(const std::string& source_name);
};

const char* stopReasonName(Executor::StopReason reason);
