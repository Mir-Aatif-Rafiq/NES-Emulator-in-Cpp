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


/*
so how everything is happening is that in memory there will be a pgm

lets assume that PC is pointing to the first inst

so it takes it , then we index it in the opcode_index vector 
figure out the addressing mode for it
call the addressing mode function and let it do what ever to the 
operand and return whatever it need to return to the opcode
and then we run the opcode function and increment pc along the way
and repeat.

*/
bool CPU6502::IMP() {  // Implied is just one byte instructions
	PC++;
	return 0;

}

bool CPU6502::IMM() {   // in assembly LDA  #$42  , uses # before value, loads A with hex val 42
														// depending on the addressing mode of LDA it will have differernt opcodes.	
														// clock cycles of the whole instruction regardless of opcode = 2 
	data_fetched = bus->bus_read(PC + 1); // now this data _fetched can be directly accessed by the opcode 
																				// bc its the part of the class.
	PC += 2;	

	return 0;

	// clearing of the data_fetched is the responsibility of the opcode

}


/*
* in 6502 the pages are indexed using the upper byte and the offset is the lower byte.
*/
bool CPU6502::ZP0() {  // its just the first page. operand is offeset of ZP

	abs_addr_fetched = 0x0000 + bus->bus_read(PC + 1); // 8bit is extended to 16bit without msb extention
	data_fetched = bus->bus_read(abs_addr_fetched);
 	PC+=2;
	return 0;
}
bool CPU6502::ZPX() {
	// if page crossed theres a wrap around condition within the zeroth page
	// addition of 0xff+0xff will still be in 1st page maths checks out

	abs_addr_fetched =(X + bus->bus_read(PC + 1)) & 0x00FF; // &0xFF handle wrap around
	data_fetched = bus->bus_read(abs_addr_fetched);
	
	PC+=2;
	return 0;
}  // its just the first page

// i think remove if and & with 0xFF regardless of the situation''''''''''''''
bool CPU6502::ZPY() {

	abs_addr_fetched =(Y + bus->bus_read(PC + 1)) & 0x00FF;
	data_fetched = bus->bus_read(abs_addr_fetched);
	
	PC+=2;
	return 0;
}
// ZPX and ZPY are zero page indexed with contents of X and Y register

// similarly with ABX and ABY , some address then offset with X and Y reg

bool CPU6502::ABS() { // e.x. LDA $8080
// in ABS if the lo byte is 0xXXFF the high byte will be taken from 
// 0xX(X+1)FF noramlly crossing page boundary without 
// triggering extra clock cycle
	lo_data_fetched = bus->bus_read(PC + 1);
	hi_data_fecthed = bus->bus_read(PC + 2);
	abs_addr_fetched = (hi_data_fecthed << 8) | lo_data_fetched;// yeah it works(shifting) without making hi_data_fetched 16 bit
	data_fetched = bus->bus_read(abs_addr_fetched);
	PC += 3;

	return 0;
}
bool CPU6502::ABX() {
	lo_data_fetched = bus->bus_read(PC + 1);
	hi_data_fecthed = bus->bus_read(PC + 2);
	abs_addr_fetched = (hi_data_fecthed << 8) | lo_data_fetched;
	abs_addr_fetched += X;
	data_fetched = bus->bus_read(abs_addr_fetched);
	PC += 3;
	if((abs_addr_fetched & 0xFF00) != (hi_data_fecthed<<8)){
		// means page boundary has been crossed so addition clock cycle
		return 1;
	}
	else
	return 0;
}

bool CPU6502::ABY() {
	lo_data_fetched = bus->bus_read(PC + 1);
	hi_data_fecthed = bus->bus_read(PC + 2);
	abs_addr_fetched = (hi_data_fecthed << 8) | lo_data_fetched;
	abs_addr_fetched += Y;
	data_fetched = bus->bus_read(abs_addr_fetched);
	PC += 3;
	if((abs_addr_fetched & 0xFF00) != (hi_data_fecthed<<8)){
		// means page boundary has been crossed so addition clock cycle
		return 1;
	}
	else 
	return 0;

}

// Note: The next 3 address modes use indirection (aka Pointers!)

/* Address Mode: Indirect
* The supplied 16-bit address is read to get the actual 16-bit address. This is
* instruction is unusual in that it has a bug in the hardware! To emulate its
* function accurately, we also need to emulate this bug. If the low byte of the
* supplied address is 0xFF, then to read the high byte of the actual address
* we need to cross a page boundary. This doesnt actually work on the chip as 
* designed, instead it wraps back around in the same page, yielding an 
* invalid actual address
*/ 

bool CPU6502::IND() { // indirect mode is pointer only for jumps
																	//  jmp ($2000) : jumps to the location pointed by 
																	// values of address $2000 and $2001. with val of 2001 being upper byte
	lo_data_fetched = bus->bus_read(PC + 1);
	hi_data_fecthed = bus->bus_read(PC + 2);
	rel_addr_fetched = (hi_data_fecthed << 8 )| lo_data_fetched;

	if(lo_data_fetched == 0xFF){ // trigger CPU bug of wrap around
		lo_data_fetched = bus->bus_read(rel_addr_fetched);
		hi_data_fecthed = bus->bus_read(rel_addr_fetched & 0xFF00);
	}

	else {
		lo_data_fetched = bus->bus_read(rel_addr_fetched);
		hi_data_fecthed = bus->bus_read(rel_addr_fetched + 1);
	}
	abs_addr_fetched = (hi_data_fecthed << 8) | lo_data_fetched;

	data_fetched = bus->bus_read(abs_addr_fetched); // i dont think we have to fetch anything?
	PC += 3;

	return 0;
}   


// i think here we also have to tackle the conditn of lo_byte+X being 0xFF
// SOLVED : it wraps around
bool CPU6502::IZX() {  // (zero page offset + x reg) value as pointer.
																		// adding first and then dereference

																		// e.g. LDA ($70,X)

	rel_addr_fetched = (0x0000 + bus->bus_read(PC + 1) + X) & 0x00FF;
																					 // even if wrap around happens,
																					 // it will get handled

  if(rel_addr_fetched & 0xFF == 0xFF){ // trigger CPU bug of wrap around
		lo_data_fetched = bus->bus_read(rel_addr_fetched);
		hi_data_fecthed = bus->bus_read(rel_addr_fetched & 0xFF00);
	}

	else {
		lo_data_fetched = bus->bus_read(rel_addr_fetched);
		hi_data_fecthed = bus->bus_read(rel_addr_fetched + 1);
	}

	abs_addr_fetched = (hi_data_fecthed << 8) | lo_data_fetched;
	
	data_fetched = bus->bus_read(abs_addr_fetched);

	PC += 2;

	return 0;

}

bool  CPU6502::IZY() { // Post indexed mode first derefernce than add Y 
																		// only works with Y reg
																		// e.g. LDA ($79),Y
	rel_addr_fetched = 0x0000 + bus->bus_read(PC + 1);
																					 // even if wrap around happens,
																					 // it will get handled


	lo_data_fetched = bus->bus_read(rel_addr_fetched);
	hi_data_fecthed = bus->bus_read(rel_addr_fetched + 1);

	abs_addr_fetched = (hi_data_fecthed << 8) | lo_data_fetched;
	abs_addr_fetched += Y;

	data_fetched = bus->bus_read(abs_addr_fetched);
	
	PC+=2;

	if((abs_addr_fetched & 0xFF00) != (hi_data_fecthed<<8)){ // boundary crossed
		return 1;
	}
	else
	return 0;

}

bool  CPU6502::REL() { // relative wrt PC but signed 2s compliment 
																		// max branch possible 127 bytes
																		// remember to minus the PC -1 before adding offset.
	lo_data_fetched = bus->bus_read(PC + 1);

	if(lo_data_fetched & 0x80 != 0){
		u_int16_t temp = (u_int16_t)lo_data_fetched;
		temp |= 0xFF00; // extending to 16
		temp = ~(temp - 1); // 2s comp calc
		rel_addr_fetched = (PC + 1 - temp) & 0xFF;
	}

	else {
		rel_addr_fetched = (PC + 1 + lo_data_fetched) & 0x00FF;
	}
	PC +=2;
	return 0;
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




void CPU6502::executor(){

	while(){

	}
}