/*
 * CPU : 6502
 */




#include <iostream>
#include <vector>
#include "cpu.h"

class CPU6502 {
	public: 
  	u_int8_t A = 0x00;  // Accumulator
  	u_int8_t X = 0x00;
  	u_int8_t Y = 0x00;
  	u_int8_t SP = 0x00; // 0100 - 01FF  stack is only of 256 bytes
  	          // SP holds the lower 8 bits of the address( top-down stack)
							// overflow is not detected by the CPU
		u_int16_t PC = 0x0000;
  	u_int8_t FLAG = 0x00; // Status register  
		/*
		* 0 bit : carry flag
		* 1 bit : zero bit
		* 2 bit : interrupt disable 
		* 3 bit : decimal mode
		* 4 bit : break 
		* 5 bit : dont care
		* 6 bit : overflow
		* 7 bit : negative 
		*/

		Clock clk;
};

class Bus {
	public: 
		bool write_enable;
		bool read_enable;
		CPU6502 cpu6502;
		RAM ram;

		void bus_write(u_int8_t data);
		u_int8_t bus_read();
		void read_enable();
		void write_enable();
};

class RAM {
	public:
		int size = 0xFFFF;
		std::vector<u_int16_t> ram;

		RAM(){
			ram = std::vector<u_int16_t>(size,0x0000);
		}
};



class AddressingModes{
	u_int8_t IMP();	u_int8_t IMM();	
	u_int8_t ZP0();	u_int8_t ZPX();	
	u_int8_t ZPY();	u_int8_t REL();
	u_int8_t ABS();	u_int8_t ABX();	
	u_int8_t ABY();	u_int8_t IND();	
	u_int8_t IZX();	u_int8_t IZY();
};

u_int8_t  AddressingModes::IMP() {  // Implied is just one byte instructions
	PC++;

}

u_int8_t AddressingModes::IMM() {   // in assemble LDA  #$42  , uses # before value, loads A with hex val 42
																		// depending on the addressing mode of LDA it will have differernt opcodes.	
	PC+=2;	

}


/*
* in 6502 the pages are indexed using the upper byte and the offset is the lower byte.
*/
u_int8_t AddressingModes::ZP0() {  // its just the first page. operand is offeset of ZP

	PC+=2;


}
// ZPX and ZPY are zero page indexed with contents of X and Y register

// similarly with ABX and ABY , some address then offset with X and Y reg

u_int8_t AddressingModes::IND() { // indirect mode is pointer only for jumps
																	//  jmp ($2000) : jumps to the location pointed by 
																	// values of address $2000 and $2001. with val of 2001 being upper byte

}   

u_int8_t AddressingModes::IZX() {  // zero page offset + x reg value as pointer.
																		// adding first and then dereference

}

u_int8_t  AddressingModes::IZY() { // Post indexed mode first derefernce than add Y 
																		// only works with Y reg

}

u_int8_t  AddressingModes::REL() { // relative wrt PC but signed 2s compliment 
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