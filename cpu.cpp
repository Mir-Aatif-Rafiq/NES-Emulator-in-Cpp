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

/*
* for now i dont see any use of creating a class for different addressing
* modes bc in older chips different addressing modes were predefined in 
* the opcodes itself.
*/
class Opcodes {

	u_int8_t ADC();	u_int8_t AND();	u_int8_t ASL();	u_int8_t BCC();
	u_int8_t BCS();	u_int8_t BEQ();	u_int8_t BIT();	u_int8_t BMI();
	u_int8_t BNE();	u_int8_t BPL();	u_int8_t BRK();	u_int8_t BVC();
	u_int8_t BVS();	u_int8_t CLC();	u_int8_t CLD();	u_int8_t CLI();
	u_int8_t CLV();	u_int8_t CMP();	u_int8_t CPX();	u_int8_t CPY();
	u_int8_t DEC();	u_int8_t DEX();	u_int8_t DEY();	u_int8_t EOR();
	u_int8_t INC();	u_int8_t INX();	u_int8_t INY();	u_int8_t JMP();
	u_int8_t JSR();	u_int8_t LDA();	u_int8_t LDX();	u_int8_t LDY();
	u_int8_t LSR();	u_int8_t NOP();	u_int8_t ORA();	u_int8_t PHA();
	u_int8_t PHP();	u_int8_t PLA();	u_int8_t PLP();	u_int8_t ROL();
	u_int8_t ROR();	u_int8_t RTI();	u_int8_t RTS();	u_int8_t SBC();
	u_int8_t SEC();	u_int8_t SED();	u_int8_t SEI();	u_int8_t STA();
	u_int8_t STX();	u_int8_t STY();	u_int8_t TAX();	u_int8_t TAY();
	u_int8_t TSX();	u_int8_t TXA();	u_int8_t TXS();	u_int8_t TYA();

	u_int8_t XXX(); // Placeholder for illegal opcode
};

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