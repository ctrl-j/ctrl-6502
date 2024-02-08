#ifndef __MEM_H__
#define __MEM_H__

#include "cpu.h"
#include "./INCLUDES/cJSON.h"


// Returns array of size WIDTH bytes of memory content
// Returns ${ADDR} through ${ADDR + WIDTH}
HBYTE* MEM_PRINT(MEM* MEMORY, ADDRESS ADDR, HBYTE WIDTH, HBYTE BLOCK_WIDTH);

// Returns byte at location $(ADDRESS)
HBYTE MEM_READ(MEM* MEMORY, ADDRESS ADDR);

// Can write up to 256 bytes of data at a time. Returns number of bytes 
// written successfully, returns -1 if the initial address is within 
// reserved memory or other errors.
HBYTE MEM_WRITE_BULK(MEM* MEMORY, ADDRESS ADDR, HBYTE* DATA, int WIDTH);

// Writes one byte of data to location specified by ADDR
// Returns address of write operation
ADDRESS MEM_WRITE(MEM* MEMORY, ADDRESS ADDR, HBYTE DATA);

// Populates memory with necessary content
// Returns status of memory initialization
HBYTE MEM_INIT(MEM* SYSMEM, cJSON* MEM_CONFIG);

// Pushes one byte to the stack, returns address written to in *WRITTEN
void STACK_PUSH(MEM* SYSMEM, CPU* SYS, HBYTE DATA, ADDRESS* WRITTEN);

// Pops one byte from stack, returns data
HBYTE STACK_POP(MEM* SYSMEM, CPU* SYS, ADDRESS* WRITTEN);

// Concatenates array of two 8-bit hex numbers into a 16-bit address
ADDRESS ADDR_CONCAT(HBYTE* DATA);

// Splits 16-bit address into two 8-bit HBYTE
// [HIGH BIT, LOW BIT]
HBYTE* ADDR_SPLIT(ADDRESS DATA);

#endif