#include "cpu.h"
#ifndef __ASSEMBLER_H__
#define __ASSEMBLER_H__

// Convert .txt file containing hex dump of machine code
// into binary file that can be loaded+run
void TXT_TO_BIN(char* TXT_FILENAME, char* BIN_FILENAME);

void CSV_PSEUDO_TO_BIN();

// Flashes system RAM from binary source
//      -MEMORY (MEM*)
//          structure containing memory HBYTE array
//      -SRC (char*)
//          filename of binary file containing machine code
//      -SRC_START (ADDRESS)
//          Line of SRC to start loading code from. Must be < SRC_WIDTH
//      -SRC_WIDTH (ADDRESS)
//          0: Loads entire SRC file
//          1 - (MAX ROM WIDTH): Loads as many bytes from SRC as it can,
//                               up to SRC_WIDTH
//          FFFF: flashes entire 256 pages including system mem and BIOS
//                Loads default BIOS and system mem values
//                Ignores other options except MEMORY, SRC
//      -MEM_START (ADDRESS)
//          Desired start location of write to RAM
//      -PRESERVE (bool)
//          0: write 0's to rest of RAM
//          1: preserve RAM other than flashed memory
void MEM_LOAD(MEM* MEMORY, char* SRC,
                   ADDRESS SRC_START, ADDRESS SRC_WIDTH,
                   ADDRESS MEM_START, bool PRESERVE);

// Exports portion of system memory to [DST].le binary file
// Loads {$MEM_START - $(MEM_START + MEM_WIDTH)}
// If MEM_WIDTH = 0, contents are copied until end of memory
void MEM_TO_BIN(MEM* MEMORY, char* DST,
                ADDRESS MEM_START, ADDRESS MEM_WIDTH);

INSTR* PARSE();


#endif