# 6502 CPU Implementation in C++

This project is a small implementation of the MOS 6502 processor. It reads raw
machine-code bytes from memory, decodes them through a 256-entry opcode table
and updates the CPU state one instruction at a time.

The repository started as a fork of
[Mir-Aatif-Rafiq/NES-Emulator-in-Cpp](https://github.com/Mir-Aatif-Rafiq/NES-Emulator-in-Cpp).
My work on the fork is focused only on completing and testing the standalone
6502 CPU. It is not an NES emulator.

## What is implemented

- Accumulator, X and Y registers
- Program counter, stack pointer and processor status register
- 64 KB byte-addressable memory connected through a bus
- The addressing modes used by the official instruction set
- A 256-entry opcode lookup table
- Arithmetic, logic, load/store, branch, transfer and stack instructions
- Binary and packed-BCD arithmetic for `ADC` and `SBC`
- Reset, IRQ, NMI, BRK and RTI behaviour
- Instruction cycle counting, including branch and page-crossing penalties

Unofficial opcodes are currently treated as either `NOP` or an unsupported
instruction. The implementation tracks instruction cycle counts, but it does
not try to reproduce every bus action on every individual clock edge.

## My contribution to the fork

I picked up the unfinished CPU branch and worked through it as a separate 6502
project. The main changes are:

- fixed the original build and memory-size problems
- connected the CPU and bus properly
- completed the instruction execution path
- implemented reset and interrupt handling
- added the missing `ADC`, `SBC` and `NOP` operations
- fixed branch offsets, zero-page pointer wrapping and several flag/register bugs
- added decimal-mode arithmetic
- added a runnable machine-code example and CPU tests

## Build and run

The project only needs a C++17 compiler and `make`.

```bash
git clone https://github.com/diaznakh/NES-Emulator-in-Cpp.git
cd NES-Emulator-in-Cpp
git checkout CPU
make run
```

The example executes:

```asm
LDA #$05
ADC #$03
STA $0200
```

Expected output:

```text
Value stored at $0200: $08
```

## Tests

```bash
make test
```

The current tests cover reset state, memory boundaries, arithmetic and flags,
BCD arithmetic, relative branches, subroutines, stack behaviour, register
transfers, BRK and RTI.

## Files

- `cpu.h` contains the CPU state, opcode table, RAM and bus interfaces.
- `cpu.cpp` contains the addressing modes and instruction behaviour.
- `main.cpp` contains a short example program written directly in machine code.
- `tests/test_cpu.cpp` contains the instruction-level tests.
- `Notes.txt` contains the original learning and design notes from the project.

## Current scope

The goal here is to understand and implement the processor itself. Cartridge
loading, graphics, audio and other NES hardware are intentionally outside the
scope of this fork.
