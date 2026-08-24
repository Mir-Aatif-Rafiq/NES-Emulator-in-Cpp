#include <iostream>
#include <string>

#include "../cpu.h"

/*
* Every test gets a fresh CPU, RAM and Bus so one program cannot leave flags or
* memory behind for the next program. Programs are written as real opcode bytes
* because that is also how a 6502 reads them from memory.
*/
class TestMachine {
public:
	CPU6502 cpu;
	RAM ram;
	Bus bus;

	TestMachine() : bus(cpu, ram) {
		bus.bus_write(0xFFFC, 0x00);
		bus.bus_write(0xFFFD, 0x80);
	}

	void load(std::initializer_list<u_int8_t> program, u_int16_t start = 0x8000) {
		u_int16_t address = start;
		for (u_int8_t byte : program) {
			bus.bus_write(address++, byte);
		}
	}
};

bool expect(bool condition, const std::string& test_name) {
	if (!condition) {
		std::cerr << "[failed] " << test_name << '\n';
		return false;
	}
	std::cout << "[passed] " << test_name << '\n';
	return true;
}

bool test_reset() {
	// Changing A before reset makes sure reset is actually restoring CPU state.
	TestMachine machine;
	machine.cpu.A = 0xAA;
	machine.cpu.reset();
	return expect(machine.cpu.PC == 0x8000 &&
		machine.cpu.SP == 0xFD &&
		machine.cpu.A == 0x00 &&
		machine.cpu.getFlag(CPU6502::I), "reset reads the vector and restores CPU state");
}

bool test_load_add_and_store() {
	// LDA #$05, ADC #$03, STA $0200
	TestMachine machine;
	machine.load({0xA9, 0x05, 0x69, 0x03, 0x8D, 0x00, 0x02});
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	machine.cpu.step();
	return expect(machine.bus.bus_read(0x0200) == 0x08 &&
		machine.cpu.cycles == 15, "load, add and store a value");
}

bool test_adc_flags() {
	// $50 + $50 produces signed overflow and a negative binary result.
	TestMachine machine;
	machine.load({0xA9, 0x50, 0x69, 0x50});
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	return expect(machine.cpu.A == 0xA0 &&
		machine.cpu.getFlag(CPU6502::V) &&
		machine.cpu.getFlag(CPU6502::N) &&
		!machine.cpu.getFlag(CPU6502::C), "ADC updates overflow, negative and carry flags");
}

bool test_sbc() {
	// SEC means there is no incoming borrow: $10 - $01 = $0F.
	TestMachine machine;
	machine.load({0xA9, 0x10, 0x38, 0xE9, 0x01});
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	machine.cpu.step();
	return expect(machine.cpu.A == 0x0F && machine.cpu.getFlag(CPU6502::C),
		"SBC uses the carry flag as inverted borrow");
}

bool test_decimal_mode() {
	// SED changes ADC/SBC from normal binary arithmetic to packed BCD arithmetic.
	TestMachine addition;
	addition.load({0xF8, 0x18, 0xA9, 0x45, 0x69, 0x55});
	addition.cpu.reset();
	for (int i = 0; i < 4; i++) {
		addition.cpu.step();
	}

	TestMachine subtraction;
	subtraction.load({0xF8, 0x38, 0xA9, 0x50, 0xE9, 0x01});
	subtraction.cpu.reset();
	for (int i = 0; i < 4; i++) {
		subtraction.cpu.step();
	}

	return expect(addition.cpu.A == 0x00 && addition.cpu.getFlag(CPU6502::C) &&
		subtraction.cpu.A == 0x49, "ADC and SBC support packed BCD values");
}

bool test_branch() {
	// BEQ jumps over LDA #$FF and continues at LDA #$2A.
	TestMachine machine;
	machine.load({0xA9, 0x00, 0xF0, 0x02, 0xA9, 0xFF, 0xA9, 0x2A});
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	machine.cpu.step();
	return expect(machine.cpu.A == 0x2A && machine.cpu.PC == 0x8008,
		"relative branch skips the expected instruction");
}

bool test_page_crossing_cycle() {
	// $80FF + X crosses into page $81, which costs one extra cycle for LDA.
	TestMachine machine;
	machine.load({0xA2, 0x01, 0xBD, 0xFF, 0x80});
	machine.bus.bus_write(0x8100, 0x7B);
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	return expect(machine.cpu.A == 0x7B && machine.cpu.cycles == 14,
		"indexed addressing adds a cycle after crossing a page");
}

bool test_zero_page_pointer_wrap() {
	// The high pointer byte after $00FF must wrap to $0000, not read $0100.
	TestMachine machine;
	machine.load({0xA0, 0x00, 0xB1, 0xFF});
	machine.bus.bus_write(0x00FF, 0x00);
	machine.bus.bus_write(0x0000, 0x90);
	machine.bus.bus_write(0x9000, 0x42);
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	return expect(machine.cpu.A == 0x42,
		"indirect pointer wraps inside the zero page");
}

bool test_subroutine_stack() {
	// JSR enters $9000 and RTS must return to the LDA at $8003.
	TestMachine machine;
	machine.load({0x20, 0x00, 0x90, 0xA9, 0x2A});
	machine.load({0xA9, 0x11, 0x60}, 0x9000);
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	machine.cpu.step();
	machine.cpu.step();
	return expect(machine.cpu.A == 0x2A && machine.cpu.SP == 0xFD,
		"JSR and RTS preserve the return address on the stack");
}

bool test_transfer_and_decrement() {
	// This catches the old TYA A=A and DEX checking-Y bugs.
	TestMachine machine;
	machine.load({0xA0, 0x42, 0x98, 0xA2, 0x00, 0xCA});
	machine.cpu.reset();
	for (int i = 0; i < 4; i++) {
		machine.cpu.step();
	}
	return expect(machine.cpu.A == 0x42 && machine.cpu.X == 0xFF &&
		machine.cpu.getFlag(CPU6502::N), "TYA and DEX update the correct registers");
}

bool test_brk_and_rti() {
	TestMachine machine;
	machine.load({0x58, 0x00, 0x00}); // CLI, BRK, padding byte
	machine.load({0x40}, 0x9000);
	machine.bus.bus_write(0xFFFE, 0x00);
	machine.bus.bus_write(0xFFFF, 0x90);
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.step();
	const bool entered_handler = machine.cpu.PC == 0x9000;
	const u_int8_t pushed_status = machine.bus.bus_read(0x01FB);
	machine.cpu.step();
	return expect(entered_handler && machine.cpu.PC == 0x8003 &&
		(pushed_status & CPU6502::B) && !(pushed_status & CPU6502::I),
		"BRK enters the interrupt handler and RTI returns");
}

bool test_irq() {
	// CLI allows IRQ, then RTI should restore the interrupted address $8001.
	TestMachine machine;
	machine.load({0x58, 0xEA}); // CLI, NOP
	machine.load({0x40}, 0x9000);
	machine.bus.bus_write(0xFFFE, 0x00);
	machine.bus.bus_write(0xFFFF, 0x90);
	machine.cpu.reset();
	machine.cpu.step();
	machine.cpu.irq();
	const bool entered_handler = machine.cpu.PC == 0x9000;
	machine.cpu.step();
	return expect(entered_handler && machine.cpu.PC == 0x8001,
		"IRQ saves CPU state and RTI restores it");
}

bool test_last_memory_address() {
	// $FFFF was outside the old 0xFFFF-sized vector.
	TestMachine machine;
	machine.bus.bus_write(0xFFFF, 0xA5);
	return expect(machine.bus.bus_read(0xFFFF) == 0xA5,
		"RAM includes the complete 16-bit address range");
}

int main() {
	bool passed = true;
	passed &= test_reset();
	passed &= test_load_add_and_store();
	passed &= test_adc_flags();
	passed &= test_sbc();
	passed &= test_decimal_mode();
	passed &= test_branch();
	passed &= test_page_crossing_cycle();
	passed &= test_zero_page_pointer_wrap();
	passed &= test_subroutine_stack();
	passed &= test_transfer_and_decrement();
	passed &= test_brk_and_rti();
	passed &= test_irq();
	passed &= test_last_memory_address();

	if (!passed) {
		return 1;
	}
	std::cout << "All CPU tests passed.\n";
	return 0;
}
