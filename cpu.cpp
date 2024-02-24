/*
 *
 */

#include <iostream>

class Registers {
  u_int8_t A = 0x00;
  u_int8_t X = 0x00;
  u_int8_t Y = 0x00;
  u_int16_t PC = 0x0000;
  u_int8_t SP = 0x00; // 0100 - 01FF  stack is only of 256 bytes
            // SP holds the lower 8 bits of the address( top-down stack)
						// overflow is not detected by the CPU
  u_int8_t FLAG = 0x00; // Status register
};