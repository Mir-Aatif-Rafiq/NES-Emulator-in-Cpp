#include <iomanip>
#include <iostream>

#include "cpu.h"

int main() {
	CPU6502 cpu;
	RAM ram;
	Bus bus(cpu, ram);

	// Reset vector points to the small program at $8000.
	bus.bus_write(0xFFFC, 0x00);
	bus.bus_write(0xFFFD, 0x80);

	// LDA #$05, ADC #$03, STA $0200
	bus.bus_write(0x8000, 0xA9);
	bus.bus_write(0x8001, 0x05);
	bus.bus_write(0x8002, 0x69);
	bus.bus_write(0x8003, 0x03);
	bus.bus_write(0x8004, 0x8D);
	bus.bus_write(0x8005, 0x00);
	bus.bus_write(0x8006, 0x02);

	cpu.reset();
	cpu.step();
	cpu.step();
	cpu.step();

	std::cout << "Value stored at $0200: $"
		<< std::hex << std::uppercase << std::setw(2) << std::setfill('0')
		<< static_cast<int>(bus.bus_read(0x0200)) << '\n';
	return 0;
}
