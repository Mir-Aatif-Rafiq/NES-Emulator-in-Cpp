
#include <iostream>
#include <vector>




class AddressingModes{
	u_int8_t IMP();	u_int8_t IMM();	
	u_int8_t ZP0();	u_int8_t ZPX();	
	u_int8_t ZPY();	u_int8_t REL();
	u_int8_t ABS();	u_int8_t ABX();	
	u_int8_t ABY();	u_int8_t IND();	
	u_int8_t IZX();	u_int8_t IZY();
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