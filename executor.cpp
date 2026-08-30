#include "executor.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

std::string trim(const std::string& text) {
	const std::size_t first = text.find_first_not_of(" \t\r\n");
	if (first == std::string::npos) {
		return "";
	}
	const std::size_t last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}

std::string upper(std::string text) {
	std::transform(text.begin(), text.end(), text.begin(),
		[](unsigned char character) {
			return static_cast<char>(std::toupper(character));
		});
	return text;
}

bool isIdentifier(const std::string& text) {
	if (text.empty() ||
		(!std::isalpha(static_cast<unsigned char>(text.front())) &&
		 text.front() != '_')) {
		return false;
	}
	return std::all_of(text.begin() + 1, text.end(), [](unsigned char character) {
		return std::isalnum(character) || character == '_';
	});
}

std::string lineError(const std::string& source_name, std::size_t line_number,
	const std::string& message) {
	return source_name + ":" + std::to_string(line_number) + ": " + message;
}

bool tryNumber(const std::string& original, u_int16_t& value) {
	std::string text = trim(original);
	int base = 10;
	if (!text.empty() && text.front() == '$') {
		text.erase(text.begin());
		base = 16;
	} else if (text.size() > 2 && text[0] == '0' &&
		(text[1] == 'x' || text[1] == 'X')) {
		text = text.substr(2);
		base = 16;
	} else if (!text.empty() && text.front() == '%') {
		text.erase(text.begin());
		base = 2;
	}

	if (text.empty()) {
		return false;
	}

	std::size_t consumed = 0;
	try {
		const unsigned long parsed = std::stoul(text, &consumed, base);
		if (consumed != text.size() || parsed > 0xFFFF) {
			return false;
		}
		value = static_cast<u_int16_t>(parsed);
		return true;
	} catch (const std::exception&) {
		return false;
	}
}

std::vector<std::string> commaSeparated(const std::string& text) {
	std::vector<std::string> values;
	std::size_t beginning = 0;
	while (beginning <= text.size()) {
		const std::size_t comma = text.find(',', beginning);
		const std::size_t ending = comma == std::string::npos ? text.size() : comma;
		const std::string value = trim(text.substr(beginning, ending - beginning));
		if (value.empty()) {
			throw std::runtime_error("empty value in .BYTE directive");
		}
		values.push_back(value);
		if (comma == std::string::npos) {
			break;
		}
		beginning = comma + 1;
	}
	return values;
}

bool isBranch(const std::string& operation) {
	return operation == "BCC" || operation == "BCS" || operation == "BEQ" ||
		operation == "BMI" || operation == "BNE" || operation == "BPL" ||
		operation == "BVC" || operation == "BVS";
}

std::string operandValue(std::string operand) {
	operand = trim(operand);
	if (!operand.empty() && operand.front() == '#') {
		operand.erase(operand.begin());
	}
	if (!operand.empty() && operand.front() == '(') {
		operand.erase(operand.begin());
	}
	if (operand.size() >= 3) {
		const std::string suffix = upper(operand.substr(operand.size() - 3));
		if (suffix == ",X)" || suffix == "),Y") {
			operand.resize(operand.size() - 3);
			return trim(operand);
		}
	}
	if (!operand.empty() && operand.back() == ')') {
		operand.pop_back();
	}
	if (operand.size() >= 2) {
		const std::string suffix = upper(operand.substr(operand.size() - 2));
		if (suffix == ",X" || suffix == ",Y") {
			operand.resize(operand.size() - 2);
		}
	}
	return trim(operand);
}

} // namespace

Executor::Executor(CPU6502& cpu_reference, Bus& bus_reference)
	: cpu(cpu_reference), bus(bus_reference) {
}

void Executor::loadFile(const std::string& file_name) {
	std::ifstream source(file_name);
	if (!source) {
		throw std::runtime_error("could not open program file: " + file_name);
	}
	loadSource(source, file_name);
}

void Executor::loadSource(std::istream& source, const std::string& source_name) {
	program_loaded = false;
	labels.clear();
	lines.clear();
	image.clear();

	firstPass(source, source_name);
	secondPass(source_name);

	// Only write to memory after both passes succeed. A syntax error therefore
	// cannot leave half of a program loaded in RAM.
	for (const auto& byte : image) {
		bus.bus_write(byte.first, byte.second);
	}
	bus.bus_write(0xFFFC, start_address & 0x00FF);
	bus.bus_write(0xFFFD, (start_address >> 8) & 0x00FF);
	program_loaded = true;
}

void Executor::firstPass(std::istream& source, const std::string& source_name) {
	std::uint32_t current_address = 0x8000;
	bool origin_seen = false;
	bool code_seen = false;
	bool end_seen = false;
	std::string raw_line;
	std::size_t line_number = 0;

	while (std::getline(source, raw_line)) {
		line_number++;
		const std::size_t comment = raw_line.find(';');
		std::string text = trim(raw_line.substr(0, comment));
		if (text.empty()) {
			continue;
		}
		if (end_seen) {
			throw std::runtime_error(lineError(source_name, line_number,
				"code found after .END"));
		}

		const std::size_t colon = text.find(':');
		if (colon != std::string::npos) {
			const std::string label = upper(trim(text.substr(0, colon)));
			if (!isIdentifier(label)) {
				throw std::runtime_error(lineError(source_name, line_number,
					"invalid label name"));
			}
			if (!labels.emplace(label, static_cast<u_int16_t>(current_address)).second) {
				throw std::runtime_error(lineError(source_name, line_number,
					"duplicate label: " + label));
			}
			text = trim(text.substr(colon + 1));
			if (text.empty()) {
				continue;
			}
		}

		const std::size_t separator = text.find_first_of(" \t");
		const std::string operation = upper(text.substr(0, separator));
		const std::string operand = separator == std::string::npos
			? "" : trim(text.substr(separator + 1));

		if (operation == ".ORG") {
			if (origin_seen || code_seen || operand.empty()) {
				throw std::runtime_error(lineError(source_name, line_number,
					".ORG must appear once, before the program"));
			}
			u_int16_t origin = 0;
			if (!tryNumber(operand, origin) || origin >= 0xFFFA) {
				throw std::runtime_error(lineError(source_name, line_number,
					"invalid .ORG address"));
			}
			current_address = origin;
			start_address = origin;
			origin_seen = true;
			continue;
		}

		if (!origin_seen) {
			start_address = static_cast<u_int16_t>(current_address);
			origin_seen = true;
		}

		if (operation == ".END") {
			if (!operand.empty()) {
				throw std::runtime_error(lineError(source_name, line_number,
					".END does not take an operand"));
			}
			end_seen = true;
			continue;
		}

		SourceLine parsed;
		parsed.number = line_number;
		parsed.operation = operation;
		parsed.operand = operand;
		parsed.address = static_cast<u_int16_t>(current_address);

		if (operation == ".BYTE") {
			try {
				parsed.size = commaSeparated(operand).size();
			} catch (const std::runtime_error& error) {
				throw std::runtime_error(lineError(source_name, line_number, error.what()));
			}
			parsed.byte_directive = true;
		} else {
			parsed.mode = selectMode(operation, operand, line_number);
			findOpcode(operation, parsed.mode, line_number);
			switch (parsed.mode) {
				case AddressMode::IMP: parsed.size = 1; break;
				case AddressMode::IMM:
				case AddressMode::ZP0:
				case AddressMode::ZPX:
				case AddressMode::ZPY:
				case AddressMode::REL:
				case AddressMode::IZX:
				case AddressMode::IZY: parsed.size = 2; break;
				default: parsed.size = 3; break;
			}
		}

		// The last six bytes contain the NMI, reset and IRQ vectors. The loader
		// reserves them instead of letting source code overwrite its own reset data.
		if (current_address + parsed.size > 0xFFFA) {
			throw std::runtime_error(lineError(source_name, line_number,
				"program overlaps the interrupt-vector area at $FFFA-$FFFF"));
		}
		lines.push_back(parsed);
		current_address += parsed.size;
		code_seen = true;
	}

	if (!code_seen) {
		throw std::runtime_error(source_name + ": program contains no instructions or bytes");
	}
	end_address = static_cast<u_int16_t>(current_address);
}

void Executor::secondPass(const std::string& source_name) {
	for (const SourceLine& line : lines) {
		u_int16_t address = line.address;
		if (line.byte_directive) {
			for (const std::string& token : commaSeparated(line.operand)) {
				const u_int16_t value = resolveValue(token, line.number);
				if (value > 0xFF) {
					throw std::runtime_error(lineError(source_name, line.number,
						".BYTE value does not fit in one byte"));
				}
				image.emplace_back(address++, static_cast<u_int8_t>(value));
			}
			continue;
		}

		image.emplace_back(address++, findOpcode(line.operation, line.mode, line.number));
		if (line.mode == AddressMode::IMP) {
			continue;
		}

		const u_int16_t value = resolveValue(operandValue(line.operand), line.number);
		if (line.mode == AddressMode::REL) {
			const int next_instruction = static_cast<int>(line.address) + 2;
			const int offset = static_cast<int>(value) - next_instruction;
			if (offset < -128 || offset > 127) {
				throw std::runtime_error(lineError(source_name, line.number,
					"branch target is outside the signed 8-bit range"));
			}
			image.emplace_back(address, static_cast<u_int8_t>(offset & 0xFF));
			continue;
		}

		const bool one_byte_operand = line.mode == AddressMode::IMM ||
			line.mode == AddressMode::ZP0 || line.mode == AddressMode::ZPX ||
			line.mode == AddressMode::ZPY || line.mode == AddressMode::IZX ||
			line.mode == AddressMode::IZY;
		if (one_byte_operand) {
			if (value > 0xFF) {
				throw std::runtime_error(lineError(source_name, line.number,
					"operand does not fit in one byte"));
			}
			image.emplace_back(address, static_cast<u_int8_t>(value));
		} else {
			// 6502 instructions store a 16-bit address low byte first.
			image.emplace_back(address++, static_cast<u_int8_t>(value & 0x00FF));
			image.emplace_back(address, static_cast<u_int8_t>((value >> 8) & 0x00FF));
		}
	}
}

Executor::AddressMode Executor::selectMode(const std::string& operation,
	const std::string& operand, std::size_t line_number) const {
	if (operand.empty()) {
		return AddressMode::IMP;
	}
	const std::string compact = upper(trim(operand));
	if (compact == "A" && (operation == "ASL" || operation == "LSR" ||
		operation == "ROL" || operation == "ROR")) {
		return AddressMode::IMP;
	}
	if (isBranch(operation)) {
		return AddressMode::REL;
	}
	if (operand.front() == '#') {
		return AddressMode::IMM;
	}

	if (compact.front() == '(') {
		if (compact.size() >= 4 && compact.substr(compact.size() - 3) == ",X)") {
			return AddressMode::IZX;
		}
		if (compact.size() >= 4 && compact.substr(compact.size() - 3) == "),Y") {
			return AddressMode::IZY;
		}
		if (compact.back() == ')') {
			return AddressMode::IND;
		}
		throw std::runtime_error("line " + std::to_string(line_number) +
			": malformed indirect operand");
	}

	AddressMode zero_page_mode = AddressMode::ZP0;
	AddressMode absolute_mode = AddressMode::ABS;
	std::string value_text = compact;
	if (compact.size() >= 2 && compact.substr(compact.size() - 2) == ",X") {
		zero_page_mode = AddressMode::ZPX;
		absolute_mode = AddressMode::ABX;
		value_text = trim(compact.substr(0, compact.size() - 2));
	} else if (compact.size() >= 2 && compact.substr(compact.size() - 2) == ",Y") {
		zero_page_mode = AddressMode::ZPY;
		absolute_mode = AddressMode::ABY;
		value_text = trim(compact.substr(0, compact.size() - 2));
	}

	u_int16_t numeric_value = 0;
	if (tryNumber(value_text, numeric_value) && numeric_value <= 0xFF &&
		hasOpcode(operation, zero_page_mode)) {
		return zero_page_mode;
	}
	return absolute_mode;
}

u_int8_t Executor::findOpcode(const std::string& operation, AddressMode mode,
	std::size_t line_number) const {
	bool (CPU6502::*wanted_mode)() = nullptr;
	switch (mode) {
		case AddressMode::IMP: wanted_mode = &CPU6502::IMP; break;
		case AddressMode::IMM: wanted_mode = &CPU6502::IMM; break;
		case AddressMode::ZP0: wanted_mode = &CPU6502::ZP0; break;
		case AddressMode::ZPX: wanted_mode = &CPU6502::ZPX; break;
		case AddressMode::ZPY: wanted_mode = &CPU6502::ZPY; break;
		case AddressMode::REL: wanted_mode = &CPU6502::REL; break;
		case AddressMode::ABS: wanted_mode = &CPU6502::ABS; break;
		case AddressMode::ABX: wanted_mode = &CPU6502::ABX; break;
		case AddressMode::ABY: wanted_mode = &CPU6502::ABY; break;
		case AddressMode::IND: wanted_mode = &CPU6502::IND; break;
		case AddressMode::IZX: wanted_mode = &CPU6502::IZX; break;
		case AddressMode::IZY: wanted_mode = &CPU6502::IZY; break;
	}

	for (std::size_t index = 0; index < cpu.opcode_lookup.size(); index++) {
		const CPU6502::Instruction& instruction = cpu.opcode_lookup[index];
		if (instruction.opcode_name == operation &&
			instruction.addrModeName == wanted_mode) {
			return static_cast<u_int8_t>(index);
		}
	}
	throw std::runtime_error("line " + std::to_string(line_number) +
		": unsupported instruction or addressing mode: " + operation);
}

bool Executor::hasOpcode(const std::string& operation, AddressMode mode) const {
	try {
		findOpcode(operation, mode, 0);
		return true;
	} catch (const std::runtime_error&) {
		return false;
	}
}

u_int16_t Executor::resolveValue(const std::string& original,
	std::size_t line_number) const {
	const std::string token = upper(trim(original));
	u_int16_t number = 0;
	if (tryNumber(token, number)) {
		return number;
	}
	if (!isIdentifier(token)) {
		throw std::runtime_error("line " + std::to_string(line_number) +
			": invalid number or label: " + original);
	}
	const auto label = labels.find(token);
	if (label == labels.end()) {
		throw std::runtime_error("line " + std::to_string(line_number) +
			": undefined label: " + token);
	}
	return label->second;
}

Executor::Result Executor::run(std::size_t maximum_instructions) {
	if (!program_loaded) {
		throw std::runtime_error("no program has been loaded");
	}
	if (maximum_instructions == 0) {
		throw std::runtime_error("maximum instruction count must be greater than zero");
	}

	cpu.reset();
	std::size_t executed = 0;
	while (executed < maximum_instructions) {
		// .END and end-of-file are host-side stop markers. They do not consume a
		// byte and do not change the real meaning of the 6502 BRK instruction.
		if (cpu.PC == end_address) {
			return {StopReason::EndOfProgram, executed, cpu.cycles, cpu.PC};
		}
		if (cpu.PC < start_address || cpu.PC > end_address) {
			std::ostringstream message;
			message << "program counter left the loaded program at $"
				<< std::hex << std::uppercase << std::setw(4) << std::setfill('0')
				<< cpu.PC;
			throw std::runtime_error(message.str());
		}
		cpu.step();
		executed++;
	}
	return {StopReason::InstructionLimit, executed, cpu.cycles, cpu.PC};
}

u_int16_t Executor::startAddress() const {
	return start_address;
}

u_int16_t Executor::endAddress() const {
	return end_address;
}

const char* stopReasonName(Executor::StopReason reason) {
	return reason == Executor::StopReason::EndOfProgram
		? "end of program" : "instruction limit";
}
