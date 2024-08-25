
#include <iostream>
#include <vector>
#include <functional>

class CPU6502 {
	public: 
  	u_int8_t A = 0x00;  // Accumulator
  	u_int8_t X = 0x00;
  	u_int8_t Y = 0x00;
  	u_int8_t SP = 0x00; // 0100 - 01FF  stack is only of 256 bytes
  	          // SP holds the lower 8 bits of the address( top-down stack)
							// overflow is not detected by the CPU
		u_int16_t PC = 0x0000;
  	u_int8_t STATUS = 0x00; // Status register  
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

	private: // ADDRESSING MODES
		u_int8_t IMP();	u_int8_t IMM();	//implicit like CLC(no operand)// immediate
		u_int8_t ZP0();	u_int8_t ZPX();	//zero page // zero page + X register
		u_int8_t ZPY();	u_int8_t REL(); //zero page +Y register //Relative for branch
		u_int8_t ABS();	u_int8_t ABX();	// absolute for 16 bit addr // addr + X reg
		u_int8_t ABY();	u_int8_t IND();	// addr+Y reg // Indirect (memory indirect)
		u_int8_t IZX();	u_int8_t IZY(); // indirect indexed + X,Y reg
	
	private: // OPCODES 
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

	class Instruction{
		private: 
		// return type (*funcptrname)(arguments);`
		// e.g. funcptrname = add;
		// we can also use auto funcptrname = add; directly
		std::string opcode_name;
		// std::function<u_int8_t(void)> opcode_name;
		u_int8_t (CPU6502::*opcodeName)(void) = nullptr;
		u_int8_t (CPU6502::*addrModeName)(void) = nullptr;
		u_int8_t clock_cycles;

	};
};

class RAM {
	public:
		int size = 0xFFFF;
		std::vector<u_int16_t> ram;

		RAM(){
			ram = std::vector<u_int16_t>(size,0x0000);
		}
};

class Bus {
	private:
		CPU6502* b_cpu6502;
		RAM* b_ram;
	public: 
		Bus(CPU6502& cpu, RAM& ram);
		bool write_enable;
		bool read_enable;
		

		void bus_write(u_int16_t address, u_int8_t data);
		u_int8_t bus_read(u_int16_t address);
		// void read_enable();   // not sure we are even gonna emulate these two
		// void write_enable();
};



/*
* for now i dont see any use of creating a class for different addressing
* modes bc in older chips different addressing modes were predefined in 
* the opcodes itself.
* 
* now after looking for it we got to know that we do need address modes
* there are a need of them there are not only 56 opcode there are
* 56 distinct opcodes.
*
* its different form 8085 which had addressing modes inside the opcode
* like LDA for address mode and LDI for immediate mode
* but in 6502 its different . LDA can have an addr or an immediate. 
* also it will help with the parsing of the instructions it will tell us
* how many bits we need to get out of the memory.
*/