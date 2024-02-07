# ctrl-6502
An MOS 6502 system emulation suite, written in C and x86 assembly.

# Features
- ctrl-6052 Emulator
  - Written in C
  - Terminal and GUI modes
    - GUI includes 256x240 system display, terminal, and memory viewer
    - Includes sound output
  - Includes BIOS code to do some simple stuff
  - Can read and write binary files for programs/memory dump
- ctrlOS System
  - 6502 "operating system", written in x86 assembly
  - Based on ctrl-6502 C-implementation
  - Able to directly read 6502 assembly and "re-assemble" into equivalent x86 code

# Specifications
- 2 KB System RAM
- 8 KB Program RAM
- 32 KB Program ROM

# Dependencies
- User interface, sound, KB + mouse input: [raylib (Main website)](https://www.raylib.com/) [raylib (GitHub)](https://github.com/raysan5/raylib)
- JSON parsing for system configuration: [cJSON](https://github.com/DaveGamble/cJSON)
