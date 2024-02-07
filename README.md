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
- chronOS System
  - Current state is just a 6502 "operating system", written in x86 assembly
  - Eventually going to do much, much more
  - Based on ctrl-6502 C-implementation
  - Able to directly read 6502 assembly and "re-assemble" into equivalent x86 code

# Specifications
## Memory Map - 64 KB total addressable memory

| Address Range | Width | Usage |
| :---: | :---: | :---: |
| `$0000 - $00FF`: **Zero page memory** |
|`$0000 - $000F`| 16 B | Local variables, function arguments |
|`$0010 - $00FF`| 240 B | Global variables, pointer tables |
| `$0100 - $01FF`: Stack memory |
|`$0100 - $019F`| 160 B | Nametable data to be copied during VBLANK |
|`$01A0 - $01FF`| 96 B | Stack memory |
| `$0200 - $07FF`: Other internal RAM |
|`$0200 - $00FF`| 256 B | OAM data to be copied during VBLANK |
|`$0300 - $00FF`| 256 B | Sound player and other variables |
|`$0400 - $07FF`| 1024 B | Arrays, global variables that are used less often


  - 2 KB System RAM
  - 22 KB Graphics/Audio memory
  - 8 KB Program/Work RAM
  - 32 KB Program ROM with bank switching

# Dependencies
- User interface, sound, KB + mouse input: [raylib (Main website)](https://www.raylib.com/) [raylib (GitHub)](https://github.com/raysan5/raylib)
- JSON parsing for system configuration: [cJSON](https://github.com/DaveGamble/cJSON)

# References
- (NesDev Wiki)[https://www.nesdev.org/wiki/Nesdev_Wiki] - memory mapping, graphic rendering
