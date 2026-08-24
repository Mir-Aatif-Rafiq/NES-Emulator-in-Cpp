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
	// CPU keeps the address of this bus because all instruction reads and
	// writes have to go through the same memory connection.
	cpu.connectBus(*this);
}

// writing on ram
void Bus::bus_write(u_int16_t address, u_int8_t data){ // R/W are wrt to ram
	assert(b_ram!=nullptr);
	assert(address >= 0x0000 && address <= 0xFFFF );
	b_ram->ram.at(address) = data;
}

// reading from ram
u_int8_t Bus::bus_read(u_int16_t address) const {
	assert(b_ram!=nullptr);
	assert(address >= 0x0000 && address <= 0xFFFF );
	return b_ram->ram.at(address);
}

/*
* Connect the CPU to the system bus.
* The CPU does not own the bus. It only keeps a pointer to the bus made in main.
*/
void CPU6502::connectBus(Bus& system_bus) {
	bus = &system_bus;
}

// Useful before reset or execution because a CPU without a bus cannot read memory.
bool CPU6502::isConnected() const {
	return bus != nullptr;
}

/*
* RESET:
* A reset clears the general registers, places the stack pointer near the top
* of the stack page and loads PC from the reset vector stored at $FFFC-$FFFD.
* The lower byte is stored first because the 6502 is little endian.
*/
void CPU6502::reset() {
	assert(bus != nullptr);
	A = 0x00;
	X = 0x00;
	Y = 0x00;
	SP = 0xFD;
	STATUS = U | I;
	PC = static_cast<u_int16_t>(bus->bus_read(0xFFFC)) |
		(static_cast<u_int16_t>(bus->bus_read(0xFFFD)) << 8);
	data_fetched = 0x00;
	abs_addr_fetched = 0x0000;
	rel_addr_fetched = 0x0000;
	cycles = 7;
}

/*
* IRQ is a maskable hardware interrupt.
* It is ignored while the I flag is set. Otherwise the current PC and status
* are pushed to the stack and execution continues from the IRQ vector.
*/
void CPU6502::irq() {
	assert(bus != nullptr);
	if (getFlag(I)) {
		return;
	}

	bus->bus_write(0x0100 + SP--, (PC >> 8) & 0x00FF);
	bus->bus_write(0x0100 + SP--, PC & 0x00FF);
	setFlag(B, false);
	setFlag(U, true);
	bus->bus_write(0x0100 + SP--, STATUS);
	setFlag(I, true);
	PC = static_cast<u_int16_t>(bus->bus_read(0xFFFE)) |
		(static_cast<u_int16_t>(bus->bus_read(0xFFFF)) << 8);
	cycles += 7;
}

/*
* NMI follows the same stack sequence as IRQ but it cannot be disabled by the
* I flag. Its handler address is stored separately at $FFFA-$FFFB.
*/
void CPU6502::nmi() {
	assert(bus != nullptr);
	bus->bus_write(0x0100 + SP--, (PC >> 8) & 0x00FF);
	bus->bus_write(0x0100 + SP--, PC & 0x00FF);
	setFlag(B, false);
	setFlag(U, true);
	bus->bus_write(0x0100 + SP--, STATUS);
	setFlag(I, true);
	PC = static_cast<u_int16_t>(bus->bus_read(0xFFFA)) |
		(static_cast<u_int16_t>(bus->bus_read(0xFFFB)) << 8);
	cycles += 8;
}

/*
* Execute one complete instruction:
* 1. read the opcode at PC
* 2. run its addressing mode to fetch the operand/effective address
* 3. run the actual operation
* 4. add the normal cycles and a possible page-crossing cycle
*/
void CPU6502::step() {
	assert(bus != nullptr);
	opcode = bus->bus_read(PC);
	Instruction& instruction = opcode_lookup.at(opcode);
	const bool page_crossed = (this->*instruction.addrModeName)();
	cycles += instruction.clock_cycles;
	const bool operation_uses_extra_cycle = (this->*instruction.opcodeName)();
	if (page_crossed && operation_uses_extra_cycle) {
		cycles++;
	}
	setFlag(U, true);
}

/*
* Setter and Getter for flags
*/
void CPU6502::setFlag(FLAGS6502 flag, bool condition) {
  if(condition){
		STATUS |= flag;
	}
  
	else{
		STATUS &= ~flag;
	}
}

bool CPU6502::getFlag(FLAGS6502 flag) {
  if(STATUS & flag){
		return 1;
	}
	else 
	return 0;
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
	// e.g. LDA ($70,X)
	// First add X to $70, then read the two-byte address from zero page.
	// Casting pointer + 1 back to 8 bit handles the $FF -> $00 wrap around.

	const u_int8_t pointer = bus->bus_read(PC + 1) + X;
	lo_data_fetched = bus->bus_read(pointer);
	hi_data_fecthed = bus->bus_read(static_cast<u_int8_t>(pointer + 1));

	abs_addr_fetched = (hi_data_fecthed << 8) | lo_data_fetched;
	
	data_fetched = bus->bus_read(abs_addr_fetched);

	PC += 2;

	return 0;

}

bool  CPU6502::IZY() { // Post indexed mode first derefernce than add Y 
																		// only works with Y reg
																		// e.g. LDA ($79),Y
	// The pointer itself is read from zero page and Y is added only after
	// the full 16-bit address has been formed.
	const u_int8_t pointer = bus->bus_read(PC + 1);
	lo_data_fetched = bus->bus_read(pointer);
	hi_data_fecthed = bus->bus_read(static_cast<u_int8_t>(pointer + 1));

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
	// Converting the byte to int8_t gives the signed range -128 to +127.
	// PC already needs to point after the two-byte branch instruction.
	const auto offset = static_cast<std::int8_t>(lo_data_fetched);
	rel_addr_fetched = static_cast<u_int16_t>(PC + 2 + offset);
	PC +=2;
	return 0;
}




// Instruction: Add with Carry
// Function:    A = A + M + C
// Flags Out:   N, V, Z, C
// Decimal mode treats both nibbles as decimal digits instead of one binary byte.
u_int8_t CPU6502::ADC() {
	const u_int8_t carry_in = getFlag(C) ? 1 : 0;
	const u_int16_t binary_sum = static_cast<u_int16_t>(A) + data_fetched + carry_in;
	setFlag(V, (~(A ^ data_fetched) & (A ^ binary_sum) & 0x0080) != 0);

	if (getFlag(D)) {
		// Correct the lower and upper nibbles separately when a digit exceeds 9.
		u_int16_t low = (A & 0x0F) + (data_fetched & 0x0F) + carry_in;
		u_int16_t high = (A >> 4) + (data_fetched >> 4);
		if (low > 9) {
			low += 6;
			high++;
		}
		if (high > 9) {
			high += 6;
		}
		setFlag(C, high > 0x0F);
		A = static_cast<u_int8_t>((high << 4) | (low & 0x0F));
	} else {
		setFlag(C, binary_sum > 0x00FF);
		A = binary_sum & 0x00FF;
	}

	setFlag(Z, A == 0);
	setFlag(N, A & 0x80);
	return 1;
}


// Instruction: Bitwise Logic AND
// Function:    A = A & M
// Flags Out:   N, Z
u_int8_t CPU6502::AND() {
	A &= data_fetched;

	setFlag(Z ,A == 0);
	setFlag(N, A & 0x80);
	return 1;
}


// Instruction: Arithmetic Shift Left
// Function:    A = C <- (A << 1) <- 0
// Flags Out:   N, Z, C
u_int8_t CPU6502::ASL() {	
	if(opcode_lookup.at(opcode).addrModeName == &CPU6502::IMP) {
		setFlag(C, A & 0x80);
		A = A << 1;
		setFlag(Z ,A == 0);
		setFlag(N, A & 0x80);
		return 0;
	}
	else {
		setFlag(C, data_fetched & 0x80);
		data_fetched = data_fetched << 1;
		setFlag(Z ,data_fetched == 0);
		setFlag(N, data_fetched & 0x80);

		bus->bus_write(abs_addr_fetched,data_fetched);
		return 0;

	}

}

// Instruction: Branch if Carry Clear
// Function:    if(C == 0) pc = address 
u_int8_t CPU6502::BCC() {
	if (getFlag(C) == 0) {

		cycles++; // adding cycle bc in executor default clk val will always get added
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}


// Instruction: Branch if Carry Set
// Function:    if(C == 1) pc = address
u_int8_t CPU6502::BCS() {
	if (getFlag(C) == 1) {

		cycles++;
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}


// Instruction: Branch if Equal
// Function:    if(Z == 1) pc = address (prev result ==0)
u_int8_t CPU6502::BEQ() {
	if (getFlag(Z) == 1) {

		cycles++;
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}


// Instruction : Test Bits in Memory with Accumulator
// Function : bits 7 and 6 of operand are transfered to bit 7 and 6 of SR (N,V);
// the zero-flag is set according to the result of the operand AND
// the accumulator (set, if the result is zero, unset otherwise).
// This allows a quick check of a few bits at once without affecting
// any of the registers, other than the status register (SR).
u_int8_t CPU6502::BIT(){
	u_int8_t temp = A & data_fetched;
	setFlag(Z, (temp & 0x00FF) == 0x00);
	setFlag(N, data_fetched & (1 << 7));
	setFlag(V, data_fetched & (1 << 6));
	return 0;
}

// Instruction: Branch if Negative
// Function:    if(N == 1) pc = address
u_int8_t CPU6502::BMI() {
	if (getFlag(N) == 1) {
	cycles++;
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}


// Instruction: Branch if Not Equal
// Function:    if(Z == 0) pc = address
u_int8_t CPU6502::BNE() {
	if (getFlag(Z) == 0) {
	cycles++;
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}

// Instruction: Branch if Positive
// Function:    if(N == 0) pc = address
u_int8_t CPU6502::BPL()
{
	if (getFlag(N) == 0){
	cycles++;
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}

// Instruction: Break
// Function:    Program Sourced Interrupt
//			PC + 2 is pushed leaving extra byte of spacing for break mark
//			(identify the reason of BRK)
// FLAG : set I and B
u_int8_t CPU6502::BRK() {
	PC++; 

	bus->bus_write(0x0100 + SP, (PC >> 8) & 0x00FF);
	SP--;
	bus->bus_write(0x0100 + SP, PC & 0x00FF);
	SP--;

	setFlag(B, 1); 
	bus->bus_write(0x0100 + SP, STATUS);
	SP--;
	setFlag(B, 0);
	setFlag(I, 1); // mask regular IRQs while the handler is running
	// Location Break Interrupt Request Handler
	PC = (u_int16_t)bus->bus_read(0xFFFE) | ((u_int16_t)bus->bus_read(0xFFFF) << 8);
	return 0;
}

// Instruction: Branch if Overflow Clear
// Function:    if(V == 0) pc = address
u_int8_t CPU6502::BVC()
{
	if (getFlag(V) == 0){
	cycles++;
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}


// Instruction: Branch if Overflow Set
// Function:    if(V == 1) pc = address
u_int8_t CPU6502::BVS()
{
	if (getFlag(V) == 1){
	cycles++;
		if ((PC & 0xFF00) != (rel_addr_fetched & 0xFF00)) {
			cycles++;
		}
		PC = rel_addr_fetched;
	}
	return 0;
}


// Instruction: Clear Carry Flag
// Function:    C = 0
u_int8_t CPU6502::CLC()
{
	setFlag(C, false);
	return 0;
}


// Instruction: Clear Decimal Flag
// Function:    D = 0
u_int8_t CPU6502::CLD()
{
	setFlag(D, false);
	return 0;
}


// Instruction: Disable Interrupts / Clear Interrupt Flag
// Function:    I = 0
u_int8_t CPU6502::CLI()
{
	setFlag(I, false);
	return 0;
}


// Instruction: Clear Overflow Flag
// Function:    V = 0
u_int8_t CPU6502::CLV()
{
	setFlag(V, false);
	return 0;
}

// Instruction: Compare Accumulator
// Function:    C <-  (A >= M )     Z <- (A - M) == 0
// Flags Out:   N, C, Z
u_int8_t CPU6502::CMP() {
	u_int8_t temp = A - data_fetched;
	setFlag(N, temp & 0x80);
	setFlag(C, A >= data_fetched);
	setFlag(Z, temp == 0);
	return 0;
}

// Instruction: Compare X Register
// Function:    C <- X >= M      Z <- (X - M) == 0
// Flags Out:   N, C, Z
u_int8_t CPU6502::CPX() {
	u_int8_t temp = X - data_fetched;
	setFlag(N, temp & 0x80);
	setFlag(C, X >= data_fetched);
	setFlag(Z, temp == 0);
	return 0;
}

// Instruction: Compare Y Register
// Function:    C <- Y >= M      Z <- (Y - M) == 0
// Flags Out:   N, C, Z
u_int8_t CPU6502::CPY() {
	u_int8_t temp = Y - data_fetched;
	setFlag(N, temp & 0x80);
	setFlag(C, Y >= data_fetched);
	setFlag(Z, temp == 0);
	return 0;
}

// Instruction: Decrement Value at Memory Location
// Function:    M = M - 1
// Flags Out:   N, Z
u_int8_t CPU6502::DEC() {
	data_fetched = data_fetched - 1;
	bus->bus_write(abs_addr_fetched, data_fetched);
	setFlag(Z, (data_fetched) == 0x00);
	setFlag(N, data_fetched & 0x80);
	return 0;
}


// Instruction: Decrement X Register
// Function:    X = X - 1
// Flags Out:   N, Z
u_int8_t CPU6502::DEX() {
	X--;
	setFlag(Z, X == 0x00);
	setFlag(N, X & 0x80);
	return 0;
}

// Instruction: Decrement Y Register
// Function:    Y = Y - 1
// Flags Out:   N, Z
u_int8_t CPU6502::DEY() {
	Y--;
	setFlag(Z, Y == 0x00);
	setFlag(N, Y & 0x80);
	return 0;
}

// Instruction: Bitwise Logic XOR
// Function:    A = A xor M
// Flags Out:   N, Z
u_int8_t CPU6502::EOR(){
	
	A = A ^ data_fetched;	
	setFlag(Z, A == 0x00);
	setFlag(N, A & 0x80);
	return 1;
}


// Instruction: Increment Value at Memory Location
// Function:    M = M + 1
// Flags Out:   N, Z
u_int8_t CPU6502::INC(){
	data_fetched = data_fetched + 1;
	bus->bus_write(abs_addr_fetched, data_fetched);
	setFlag(Z, (data_fetched) == 0x00);
	setFlag(N, data_fetched & 0x80);
	return 0;
}


// Instruction: Increment X Register
// Function:    X = X + 1
// Flags Out:   N, Z
u_int8_t CPU6502::INX(){
	X++;
	setFlag(Z, X == 0x00);
	setFlag(N, X & 0x80);
	return 0;
}


// Instruction: Increment Y Register
// Function:    Y = Y + 1
// Flags Out:   N, Z
u_int8_t CPU6502::INY(){
	Y++;
	setFlag(Z, Y == 0x00);
	setFlag(N, Y & 0x80);
	return 0;
}

// Instruction: Jump To Location
// Function:    pc = address
u_int8_t CPU6502::JMP() {
	PC = abs_addr_fetched;
	return 0;
}

// Instruction: Jump To Sub-Routine pushing return addr on stack
// Function:    Push current pc to stack, pc = address
// first high bit is pushed then low
u_int8_t CPU6502::JSR() {
	PC--;
	bus->bus_write((u_int16_t)SP | 0x100, (PC >> 8) & 0x00FF);
	SP--;
	bus->bus_write((u_int16_t)SP | 0x100, PC & 0x00FF);
	SP--;

	PC = abs_addr_fetched;
	return 0;
}

// Instruction: Load The Accumulator
// Function:    A = M
// Flags Out:   N, Z
u_int8_t CPU6502::LDA() {
	A = data_fetched;
	setFlag(Z, A == 0x00);
	setFlag(N, A & 0x80);
	return 1;
}


// Instruction: Load The X Register
// Function:    X = M
// Flags Out:   N, Z
u_int8_t CPU6502::LDX() {
	X = data_fetched;
	setFlag(Z, X == 0x00);
	setFlag(N, X & 0x80);
	return 1;
}


// Instruction: Load The Y Register
// Function:    Y = M
// Flags Out:   N, Z
u_int8_t CPU6502::LDY() {
	Y = data_fetched;
	setFlag(Z, Y == 0x00);
	setFlag(N, Y & 0x80);
	return 1;
}

// Instruction: Shift one bit right
// Function:    0 -> [76543210] -> C // goes to carry if 0th bit 1
// Flags Out:   N, Z ,C
u_int8_t CPU6502::LSR(){
	if(opcode_lookup.at(opcode).addrModeName == &CPU6502::IMP) {
		setFlag(C, A & 0x01);
		A = A >> 1;
		setFlag(Z ,A == 0);
		setFlag(N, A & 0x80);
		return 0;
	}
	else {
		setFlag(C, data_fetched & 0x01);
		data_fetched = data_fetched >> 1;
		setFlag(Z ,data_fetched == 0);
		setFlag(N, data_fetched & 0x80);

		bus->bus_write(abs_addr_fetched,data_fetched);
		return 0;

	}
}

// Instruction: Bitwise Logic OR
// Function:    A = A | M
// Flags Out:   N, Z
u_int8_t CPU6502::ORA()
{
	A = A | data_fetched;
	setFlag(Z, A == 0x00);
	setFlag(N, A & 0x80);
	return 1;
}

u_int8_t CPU6502::NOP() {
	// No operation changes no register; PC was already updated by IMP.
	return 0;
}

// Instruction: Push Accumulator to Stack
// Function:    A -> stack
u_int8_t CPU6502::PHA() {
	bus->bus_write(0x0100+SP,A);
	SP--;
	return 0;
}


// Instruction: Push Status Register to Stack
// Function:    status -> stack
// Note:        Break flag and unused is set to 1 before push

// When an interrupt occurs, and the status register is pushed onto the stack,
// the B flag is set to 1 if the interrupt was caused by a BRK instruction, 
// and to 0 if it was a hardware interrupt.
u_int8_t CPU6502::PHP() {
	bus->bus_write(0x0100 + SP, STATUS | B | U);
	setFlag(B, 0);
	setFlag(U, 0);
	SP--;
	return 0;
}


// Instruction: Pop Accumulator off Stack
// Function:    A <- stack
// Flags Out:   N, Z
u_int8_t CPU6502::PLA() {
	SP++;
	A = bus->bus_read(0x0100 + SP);
	setFlag(Z, A == 0x00);
	setFlag(N, A & 0x80);
	return 0;
}


// Instruction: Pop Status Register off Stack
// Function:    Status <- stack
u_int8_t CPU6502::PLP() {
	SP++;
	STATUS = bus->bus_read(0x0100 + SP);
	setFlag(U, 1); // its said that its always 1
	return 0;
}

// Instruction: Rotate One Bit Left (Memory or Accumulator)
// Function:    C <- [76543210] <- C
// FLAG: C,N,Z
u_int8_t CPU6502::ROL() {
	if (opcode_lookup[opcode].addrModeName == &CPU6502::IMP){
		bool t_flag = A & 0x80;
		A = (A<<1) | getFlag(C);
		setFlag(C, t_flag);
		setFlag(Z, A == 0);
		setFlag(N, A & 0x80);
	}
	else{
	u_int16_t temp = (u_int16_t)(data_fetched << 1) | getFlag(C);
	setFlag(C, temp & 0xFF00);
	setFlag(Z, (temp & 0x00FF) == 0x0000);
	setFlag(N, temp & 0x0080);
		bus->bus_write(abs_addr_fetched, temp & 0x00FF);
	}
	return 0;
}

// Instruction: Rotate One Bit Right (Memory or Accumulator)
// Function:    C -> [76543210] -> C
// FLAG: C,N,Z
u_int8_t CPU6502::ROR() {
	if (opcode_lookup[opcode].addrModeName == &CPU6502::IMP){
		bool t_flag = A & 0x01;
		A = (A>>1) | getFlag(C)<<7;
		setFlag(C, t_flag);
		setFlag(Z, A == 0);
		setFlag(N, A & 0x80);
	}
	else{
		bool t_flag = data_fetched & 0x01;
		u_int16_t temp = (u_int16_t)(data_fetched >> 1) | getFlag(C)<<7;
		setFlag(C, t_flag);
		setFlag(Z, (temp & 0x00FF) == 0x00);
		setFlag(N, temp & 0x0080);
		bus->bus_write(abs_addr_fetched, temp & 0x00FF);
	}
	return 0;
}

// Instruction: Return From Interrupt
// Function:    The status register is pulled with the break flag
//						and bit 5 ignored. Then PC is pulled from the stack.
// PC+2 was pushed and now is read exactly like it should be.
u_int8_t CPU6502::RTI() {
	SP++;
	STATUS = bus->bus_read(0x0100 + SP);
	// B is not a physical stored flag and U is kept high inside the emulator.
	setFlag(B, false);
	setFlag(U, true);

	SP++;
	PC = (u_int16_t)bus->bus_read(0x0100 + SP);
	SP++;
	PC |= (u_int16_t)bus->bus_read(0x0100 + SP) << 8;
	return 0;
}

// Instruction: Return From SubRoutine
// can work with both the methods
// my method that the PC is completely updated in addressing modes
// so if PC was PUSHED instack instead of PC-1
// now we get PC and we are not updating so its good
u_int8_t CPU6502::RTS() {
	SP++;
	PC = (u_int16_t)bus->bus_read(0x0100 + SP);
	SP++;
	PC |= (u_int16_t)bus->bus_read(0x0100 + SP) << 8;
	
	PC++;
	
	return 0;
}

// Instruction: Subtract with Carry
// Function:    A = A - M - (1 - C)
// Flags Out:   N, V, Z, C
// Carry works as an inverted borrow on the 6502: set means no borrow.
u_int8_t CPU6502::SBC() {
	const int borrow = getFlag(C) ? 0 : 1;
	const int difference = static_cast<int>(A) - data_fetched - borrow;
	const u_int8_t binary_result = difference & 0x00FF;
	setFlag(V, ((A ^ data_fetched) & (A ^ binary_result) & 0x80) != 0);
	setFlag(C, difference >= 0);

	if (getFlag(D)) {
		// Borrow is corrected separately for the two packed decimal digits.
		int low = (A & 0x0F) - (data_fetched & 0x0F) - borrow;
		int high = (A >> 4) - (data_fetched >> 4);
		if (low < 0) {
			low -= 6;
			high--;
		}
		if (high < 0) {
			high -= 6;
		}
		A = static_cast<u_int8_t>(((high & 0x0F) << 4) | (low & 0x0F));
	} else {
		A = binary_result;
	}

	setFlag(Z, A == 0);
	setFlag(N, A & 0x80);
	return 1;
}

// Instruction: Set Carry Flag
// Function:    C = 1
u_int8_t CPU6502::SEC() {
	setFlag(C, true);
	return 0;
}


// Instruction: Set Decimal Flag
// Function:    D = 1
u_int8_t CPU6502::SED() {
	setFlag(D, true);
	return 0;
}


// Instruction: Set Interrupt Flag / Enable Interrupts
// Function:    I = 1
u_int8_t CPU6502::SEI() {
	setFlag(I, true);
	return 0;
}

// Instruction: Store Accumulator at Address
// Function:    M = A
u_int8_t CPU6502::STA() {
	bus->bus_write(abs_addr_fetched, A);
	return 0;
}


// Instruction: Store X Register at Address
// Function:    M = X
u_int8_t CPU6502::STX() {
	bus->bus_write(abs_addr_fetched, X);
	return 0;
}


// Instruction: Store Y Register at Address
// Function:    M = Y
u_int8_t CPU6502::STY() {
	bus->bus_write(abs_addr_fetched, Y);
	return 0;
}

// Instruction: Transfer Accumulator to X Register
// Function:    X = A
// Flags Out:   N, Z
u_int8_t CPU6502::TAX() {
	X = A;
	setFlag(Z, X == 0x00);
	setFlag(N, X & 0x80);
	return 0;
}


// Instruction: Transfer Accumulator to Y Register
// Function:    Y = A
// Flags Out:   N, Z
u_int8_t CPU6502::TAY() {
	Y = A;
	setFlag(Z, Y == 0x00);
	setFlag(N, Y & 0x80);
	return 0;
}


// Instruction: Transfer Stack Pointer to X Register
// Function:    X = stack pointer
// Flags Out:   N, Z
u_int8_t CPU6502::TSX() {
	X = SP;
	setFlag(Z, X == 0x00);
	setFlag(N, X & 0x80);
	return 0;
}


// Instruction: Transfer X Register to Accumulator
// Function:    A = X
// Flags Out:   N, Z
u_int8_t CPU6502::TXA() {
	A = X;
	setFlag(Z, A == 0x00);
	setFlag(N, A & 0x80);
	return 0;
}


// Instruction: Transfer X Register to Stack Pointer
// Function:    stack pointer = X
u_int8_t CPU6502::TXS() {
	SP = X;
	return 0;
}


// Instruction: Transfer Y Register to Accumulator
// Function:    A = Y
// Flags Out:   N, Z
u_int8_t CPU6502::TYA() {
	A = Y;
	setFlag(Z, A == 0x00);
	setFlag(N, A & 0x80);
	return 0;
}

// This function captures illegal opcodes
u_int8_t CPU6502::XXX() {
	return 0;
}








// reset , interrrupt request, nmi these signals are async and need to be checked.






void CPU6502::executor(){
	// Kept for compatibility with the original interface. One call runs one opcode.
	step();
}
