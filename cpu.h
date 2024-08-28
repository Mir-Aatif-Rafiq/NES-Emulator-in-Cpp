
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

    Bus* bus;

	private: // ADDRESSING MODES returning if additional clk cycle occurs
  // 0 -> no addtional clk cycle
  // 1 -> 1 addition clk cycle

		bool IMP();	bool IMM();	//implicit like CLC(no operand)// immediate
		bool ZP0();	bool ZPX();	//zero page // zero page + X register
		bool ZPY();	bool REL(); //zero page +Y register //Relative for branch
		bool ABS();	bool ABX();	// absolute for 16 bit addr // addr + X reg
		bool ABY();	bool IND();	// addr+Y reg // Indirect (memory indirect)
		bool IZX();	bool IZY(); // indirect indexed + X,Y reg
	
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

	class Instruction{  // thinking about making it virtual
		public: 
			// return type (*funcptrname)(arguments);`
			// e.g. funcptrname = add;
			// we can also use auto funcptrname = add; directly
			std::string opcode_name;
			// std::function<u_int8_t(void)> opcode_name;
			u_int8_t (CPU6502::*opcodeName)(void) = nullptr;
			bool (CPU6502::*addrModeName)(void) = nullptr;
			u_int8_t clock_cycles;
	};

	private:
    std::vector<CPU6502::Instruction> opcode_lookup = {
      {"BRK", &CPU6502::BRK, &CPU6502::IMM, 7}, {"ORA", &CPU6502::ORA, &CPU6502::IZX, 6},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 3}, {"ORA", &CPU6502::ORA, &CPU6502::ZP0, 3},
      {"ASL", &CPU6502::ASL, &CPU6502::ZP0, 5}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"PHP", &CPU6502::PHP, &CPU6502::IMP, 3}, {"ORA", &CPU6502::ORA, &CPU6502::IMM, 2},
      {"ASL", &CPU6502::ASL, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 2},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"ORA", &CPU6502::ORA, &CPU6502::ABS, 4},
      {"ASL", &CPU6502::ASL, &CPU6502::ABS, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"BPL", &CPU6502::BPL, &CPU6502::REL, 2}, {"ORA", &CPU6502::ORA, &CPU6502::IZY, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"ORA", &CPU6502::ORA, &CPU6502::ZPX, 4},
      {"ASL", &CPU6502::ASL, &CPU6502::ZPX, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"CLC", &CPU6502::CLC, &CPU6502::IMP, 2}, {"ORA", &CPU6502::ORA, &CPU6502::ABY, 4},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"ORA", &CPU6502::ORA, &CPU6502::ABX, 4},
      {"ASL", &CPU6502::ASL, &CPU6502::ABX, 7}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"JSR", &CPU6502::JSR, &CPU6502::ABS, 6}, {"AND", &CPU6502::AND, &CPU6502::IZX, 6},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"BIT", &CPU6502::BIT, &CPU6502::ZP0, 3}, {"AND", &CPU6502::AND, &CPU6502::ZP0, 3},
      {"ROL", &CPU6502::ROL, &CPU6502::ZP0, 5}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"PLP", &CPU6502::PLP, &CPU6502::IMP, 4}, {"AND", &CPU6502::AND, &CPU6502::IMM, 2},
      {"ROL", &CPU6502::ROL, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 2},
      {"BIT", &CPU6502::BIT, &CPU6502::ABS, 4}, {"AND", &CPU6502::AND, &CPU6502::ABS, 4},
      {"ROL", &CPU6502::ROL, &CPU6502::ABS, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"BMI", &CPU6502::BMI, &CPU6502::REL, 2}, {"AND", &CPU6502::AND, &CPU6502::IZY, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"AND", &CPU6502::AND, &CPU6502::ZPX, 4},
      {"ROL", &CPU6502::ROL, &CPU6502::ZPX, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"SEC", &CPU6502::SEC, &CPU6502::IMP, 2}, {"AND", &CPU6502::AND, &CPU6502::ABY, 4},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"AND", &CPU6502::AND, &CPU6502::ABX, 4},
      {"ROL", &CPU6502::ROL, &CPU6502::ABX, 7}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"RTI", &CPU6502::RTI, &CPU6502::IMP, 6}, {"EOR", &CPU6502::EOR, &CPU6502::IZX, 6},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 3}, {"EOR", &CPU6502::EOR, &CPU6502::ZP0, 3},
      {"LSR", &CPU6502::LSR, &CPU6502::ZP0, 5}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"PHA", &CPU6502::PHA, &CPU6502::IMP, 3}, {"EOR", &CPU6502::EOR, &CPU6502::IMM, 2},
      {"LSR", &CPU6502::LSR, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 2},
      {"JMP", &CPU6502::JMP, &CPU6502::ABS, 3}, {"EOR", &CPU6502::EOR, &CPU6502::ABS, 4},
      {"LSR", &CPU6502::LSR, &CPU6502::ABS, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"BVC", &CPU6502::BVC, &CPU6502::REL, 2}, {"EOR", &CPU6502::EOR, &CPU6502::IZY, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"EOR", &CPU6502::EOR, &CPU6502::ZPX, 4},
      {"LSR", &CPU6502::LSR, &CPU6502::ZPX, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"CLI", &CPU6502::CLI, &CPU6502::IMP, 2}, {"EOR", &CPU6502::EOR, &CPU6502::ABY, 4},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"EOR", &CPU6502::EOR, &CPU6502::ABX, 4},
      {"LSR", &CPU6502::LSR, &CPU6502::ABX, 7}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"RTS", &CPU6502::RTS, &CPU6502::IMP, 6}, {"ADC", &CPU6502::ADC, &CPU6502::IZX, 6},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 3}, {"ADC", &CPU6502::ADC, &CPU6502::ZP0, 3},
      {"ROR", &CPU6502::ROR, &CPU6502::ZP0, 5}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"PLA", &CPU6502::PLA, &CPU6502::IMP, 4}, {"ADC", &CPU6502::ADC, &CPU6502::IMM, 2},
      {"ROR", &CPU6502::ROR, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 2},
      {"JMP", &CPU6502::JMP, &CPU6502::IND, 5}, {"ADC", &CPU6502::ADC, &CPU6502::ABS, 4},
      {"ROR", &CPU6502::ROR, &CPU6502::ABS, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"BVS", &CPU6502::BVS, &CPU6502::REL, 2}, {"ADC", &CPU6502::ADC, &CPU6502::IZY, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"ADC", &CPU6502::ADC, &CPU6502::ZPX, 4},
      {"ROR", &CPU6502::ROR, &CPU6502::ZPX, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"SEI", &CPU6502::SEI, &CPU6502::IMP, 2}, {"ADC", &CPU6502::ADC, &CPU6502::ABY, 4},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"ADC", &CPU6502::ADC, &CPU6502::ABX, 4},
      {"ROR", &CPU6502::ROR, &CPU6502::ABX, 7}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"STA", &CPU6502::STA, &CPU6502::IZX, 6},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"STY", &CPU6502::STY, &CPU6502::ZP0, 3}, {"STA", &CPU6502::STA, &CPU6502::ZP0, 3},
      {"STX", &CPU6502::STX, &CPU6502::ZP0, 3}, {"???", &CPU6502::XXX, &CPU6502::IMP, 3},
      {"DEY", &CPU6502::DEY, &CPU6502::IMP, 2}, {"???", &CPU6502::NOP, &CPU6502::IMP, 2},
      {"TXA", &CPU6502::TXA, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 2},
      {"STY", &CPU6502::STY, &CPU6502::ABS, 4}, {"STA", &CPU6502::STA, &CPU6502::ABS, 4},
      {"STX", &CPU6502::STX, &CPU6502::ABS, 4}, {"???", &CPU6502::XXX, &CPU6502::IMP, 4},
      {"BCC", &CPU6502::BCC, &CPU6502::REL, 2}, {"STA", &CPU6502::STA, &CPU6502::IZY, 6},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"STY", &CPU6502::STY, &CPU6502::ZPX, 4}, {"STA", &CPU6502::STA, &CPU6502::ZPX, 4},
      {"STX", &CPU6502::STX, &CPU6502::ZPY, 4}, {"???", &CPU6502::XXX, &CPU6502::IMP, 4},
      {"TYA", &CPU6502::TYA, &CPU6502::IMP, 2}, {"STA", &CPU6502::STA, &CPU6502::ABY, 5},
      {"TXS", &CPU6502::TXS, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 5}, {"STA", &CPU6502::STA, &CPU6502::ABX, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 5}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"LDY", &CPU6502::LDY, &CPU6502::IMM, 2}, {"LDA", &CPU6502::LDA, &CPU6502::IZX, 6},
      {"LDX", &CPU6502::LDX, &CPU6502::IMM, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"LDY", &CPU6502::LDY, &CPU6502::ZP0, 3}, {"LDA", &CPU6502::LDA, &CPU6502::ZP0, 3},
      {"LDX", &CPU6502::LDX, &CPU6502::ZP0, 3}, {"???", &CPU6502::XXX, &CPU6502::IMP, 3},
      {"TAY", &CPU6502::TAY, &CPU6502::IMP, 2}, {"LDA", &CPU6502::LDA, &CPU6502::IMM, 2},
      {"TAX", &CPU6502::TAX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 2},
      {"LDY", &CPU6502::LDY, &CPU6502::ABS, 4}, {"LDA", &CPU6502::LDA, &CPU6502::ABS, 4},
      {"LDX", &CPU6502::LDX, &CPU6502::ABS, 4}, {"???", &CPU6502::XXX, &CPU6502::IMP, 4},
      {"BCS", &CPU6502::BCS, &CPU6502::REL, 2}, {"LDA", &CPU6502::LDA, &CPU6502::IZY, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"LDY", &CPU6502::LDY, &CPU6502::ZPX, 4}, {"LDA", &CPU6502::LDA, &CPU6502::ZPX, 4},
      {"LDX", &CPU6502::LDX, &CPU6502::ZPY, 4}, {"???", &CPU6502::XXX, &CPU6502::IMP, 4},
      {"CLV", &CPU6502::CLV, &CPU6502::IMP, 2}, {"LDA", &CPU6502::LDA, &CPU6502::ABY, 4},
      {"TSX", &CPU6502::TSX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 4},
      {"LDY", &CPU6502::LDY, &CPU6502::ABX, 4}, {"LDA", &CPU6502::LDA, &CPU6502::ABX, 4},
      {"LDX", &CPU6502::LDX, &CPU6502::ABY, 4}, {"???", &CPU6502::XXX, &CPU6502::IMP, 4},
      {"CPY", &CPU6502::CPY, &CPU6502::IMM, 2}, {"CMP", &CPU6502::CMP, &CPU6502::IZX, 6},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"CPY", &CPU6502::CPY, &CPU6502::ZP0, 3}, {"CMP", &CPU6502::CMP, &CPU6502::ZP0, 3},
      {"DEC", &CPU6502::DEC, &CPU6502::ZP0, 5}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"INY", &CPU6502::INY, &CPU6502::IMP, 2}, {"CMP", &CPU6502::CMP, &CPU6502::IMM, 2},
      {"DEX", &CPU6502::DEX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 2},
      {"CPY", &CPU6502::CPY, &CPU6502::ABS, 4}, {"CMP", &CPU6502::CMP, &CPU6502::ABS, 4},
      {"DEC", &CPU6502::DEC, &CPU6502::ABS, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"BNE", &CPU6502::BNE, &CPU6502::REL, 2}, {"CMP", &CPU6502::CMP, &CPU6502::IZY, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"CMP", &CPU6502::CMP, &CPU6502::ZPX, 4},
      {"DEC", &CPU6502::DEC, &CPU6502::ZPX, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"CLD", &CPU6502::CLD, &CPU6502::IMP, 2}, {"CMP", &CPU6502::CMP, &CPU6502::ABY, 4},
      {"NOP", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"CMP", &CPU6502::CMP, &CPU6502::ABX, 4},
      {"DEC", &CPU6502::DEC, &CPU6502::ABX, 7}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"CPX", &CPU6502::CPX, &CPU6502::IMM, 2}, {"SBC", &CPU6502::SBC, &CPU6502::IZX, 6},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"CPX", &CPU6502::CPX, &CPU6502::ZP0, 3}, {"SBC", &CPU6502::SBC, &CPU6502::ZP0, 3},
      {"INC", &CPU6502::INC, &CPU6502::ZP0, 5}, {"???", &CPU6502::XXX, &CPU6502::IMP, 5},
      {"INX", &CPU6502::INX, &CPU6502::IMP, 2}, {"SBC", &CPU6502::SBC, &CPU6502::IMM, 2},
      {"NOP", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::SBC, &CPU6502::IMP, 2},
      {"CPX", &CPU6502::CPX, &CPU6502::ABS, 4}, {"SBC", &CPU6502::SBC, &CPU6502::ABS, 4},
      {"INC", &CPU6502::INC, &CPU6502::ABS, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"BEQ", &CPU6502::BEQ, &CPU6502::REL, 2}, {"SBC", &CPU6502::SBC, &CPU6502::IZY, 5},
      {"???", &CPU6502::XXX, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 8},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"SBC", &CPU6502::SBC, &CPU6502::ZPX, 4},
      {"INC", &CPU6502::INC, &CPU6502::ZPX, 6}, {"???", &CPU6502::XXX, &CPU6502::IMP, 6},
      {"SED", &CPU6502::SED, &CPU6502::IMP, 2}, {"SBC", &CPU6502::SBC, &CPU6502::ABY, 4},
      {"NOP", &CPU6502::NOP, &CPU6502::IMP, 2}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
      {"???", &CPU6502::NOP, &CPU6502::IMP, 4}, {"SBC", &CPU6502::SBC, &CPU6502::ABX, 4},
      {"INC", &CPU6502::INC, &CPU6502::ABX, 7}, {"???", &CPU6502::XXX, &CPU6502::IMP, 7},
  	}; // 256 size , "???" for empty opcode capture
		// index of this vector is the equivalent of the opcode in HEX

    u_int8_t data_fetched = 0x00;
    u_int8_t lo_data_fetched = 0x00;
    u_int8_t hi_data_fecthed = 0x00;
    u_int16_t abs_addr_fetched = 0x0000;
    u_int16_t rel_addr_fetched = 0x0000;

  public:
    void executor();

};

/*
* the ram of 6502 has multiple segments :
  it has a zero page $0000- $00FF can address 256 bytes

  the system stack starts from the second page $0100-01FF
  and cannot be reallocated.

  The only other reserved locations in the memory map are 
  the very last 6 bytes of memory $FFFA to $FFFF which must
  be programmed with the addresses of the non-maskable 
  interrupt handler ($FFFA/B), the power on reset location ($FFFC/D)
  and the BRK/interrupt request handler ($FFFE/F) respectively.
*/

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