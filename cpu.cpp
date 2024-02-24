/*
 * CPU : 6502
 */

#include <iostream>
#include <vector>

class CPU6502 {
	public: 
  	u_int8_t A = 0x00;  // Accumulator
  	u_int8_t X = 0x00;
  	u_int8_t Y = 0x00;
  	u_int16_t PC = 0x0000;
  	u_int8_t SP = 0x00; // 0100 - 01FF  stack is only of 256 bytes
  	          // SP holds the lower 8 bits of the address( top-down stack)
							// overflow is not detected by the CPU
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