/*
 * CPU : 6502
 */




#include <iostream>
#include <vector>
#include <assert.h>
#include "cpu.h"



Bus::Bus(CPU6502& cpu, RAM& ram){
	b_cpu6502 = &cpu;
	b_ram = &ram;
}

// writing on ram
void Bus::bus_write(u_int16_t address, u_int8_t data){ // R/W are wrt to ram
	assert(b_ram!=nullptr);
	assert(address >= 0x0000 && address <= 0xFFFF );
	b_ram->ram.at(address) = data;
}

// reading from ram
u_int8_t Bus::bus_read(u_int16_t address){
	assert(b_ram!=nullptr);
	assert(address >= 0x0000 && address <= 0xFFFF );
	return b_ram->ram.at(address);
}

// should i keep seperate functions for ram r/w which bus will use???






u_int8_t  CPU6502::IMP() {  // Implied is just one byte instructions
	PC++;

}

u_int8_t CPU6502::IMM() {   // in assemble LDA  #$42  , uses # before value, loads A with hex val 42
																		// depending on the addressing mode of LDA it will have differernt opcodes.	
	PC+=2;	

}


/*
* in 6502 the pages are indexed using the upper byte and the offset is the lower byte.
*/
u_int8_t CPU6502::ZP0() {  // its just the first page. operand is offeset of ZP

	PC+=2;


}
// ZPX and ZPY are zero page indexed with contents of X and Y register

// similarly with ABX and ABY , some address then offset with X and Y reg

u_int8_t CPU6502::IND() { // indirect mode is pointer only for jumps
																	//  jmp ($2000) : jumps to the location pointed by 
																	// values of address $2000 and $2001. with val of 2001 being upper byte

}   

u_int8_t CPU6502::IZX() {  // zero page offset + x reg value as pointer.
																		// adding first and then dereference

}

u_int8_t  CPU6502::IZY() { // Post indexed mode first derefernce than add Y 
																		// only works with Y reg

}

u_int8_t  CPU6502::REL() { // relative wrt PC but signed 2s compliment 
																		// max branch possible 127 bytes
																		// remember to minus the PC -1 before adding offset.

}

/*
* currently clock has no functionality apart from keeping 
* tabs on how many clk cycles has passed and maybe it might
* be used later bc many opcodes having different overflows 
* can alter the number of clock cycles they take.
*/
class Clock {
	public: 
		u_int64_t clock_cycles = 0;

};

// reset , interrrupt request, nmi these signals are async and need to be checked.



// theory about the pages in 6502 and the need of an additional clk cycle if
// the page changes.