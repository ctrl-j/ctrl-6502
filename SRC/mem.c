/*
    Copyright (C) 2024  Joshua Jackson
    [Full GPL-3.0 license can be found in LICENSE, in the root folder]
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

// cJSON is written by Dave Gamble and other contributers under the MIT License
#include "./INCLUDES/cJSON.h"

#include "mem.h"
#include "cpu.h"
#include "util.h"
#include "assembler.h"

HBYTE* MEM_PRINT(MEM* MEMORY, ADDRESS ADDR, HBYTE WIDTH, HBYTE BLOCK_WIDTH) {
    ///////////////////////////////////////////////////
    // Check if attempting to access reserved memory
    ///////////////////////////////////////////////////
    // $0000 - $01FF for Zero Page and Stack
    // $FFFA - $FFFF for various flags
    ///////////////////////////////////////////////////
    int ind_counter = 0;
    int byte_counter = 0;

    ADDRESS ADDR_TEMP = ADDR;
    int ADDR_END = ADDR + WIDTH;

    ADDRESS PREBLOCK;

    HBYTE* MEM_LOC = ADDR_SPLIT(ADDR);
    HBYTE PG = MEM_LOC[0];
    HBYTE INDEX = MEM_LOC[1];

    char BUFF[256];
    char *MSG;

    int LINE_NUM = 1;

    // Start of byte block, no adjustment needed
    if (ADDR % (BLOCK_WIDTH) == 0) {
        ADDR_TEMP = ADDR;
        PREBLOCK = ADDR;
    }
    else {
        //Get address of highest multiple of BW below starting ADDR
        PREBLOCK = ADDR / (BLOCK_WIDTH);
        ADDR_TEMP = BLOCK_WIDTH * PREBLOCK;
    }

    if (ADDR_END > 0xFFFF) {
        logMsg(stdout, "READ ERROR: Attempted to read above $FFFF\n", 0);
        return NULL;
    }

    logMsg(stdout, "\033[4mLine #\tAddress\t\t\t\t\t\tPage, Index\n", 5);

    while (ADDR_TEMP < ADDR_END) {
        MEM_LOC = ADDR_SPLIT(ADDR_TEMP);
        PG = MEM_LOC[0];
        INDEX = MEM_LOC[1];
        if (ind_counter == 0) {
            printf("\n");
            // Address of start of line
            MSG = "#%4d\t \033[4m$%04X\033[0m\033[45;1;7m \t";

            snprintf(BUFF, 256, MSG, LINE_NUM, ADDR_TEMP);
            logMsg(stdout, BUFF, 5);
            
            if (ADDR_TEMP == ADDR) {
                MSG = "{";
                logMsg(stdout, MSG, 4);

                MSG = "%02X";
                snprintf(BUFF, 256, MSG, MEMORY->RAM[PG][INDEX]);
                logMsg(stdout, BUFF, 7);

                MSG = "}";
                logMsg(stdout, MSG, 4);
            }
            else {
                MSG = " %02X ";
                snprintf(BUFF, 256, MSG, MEMORY->RAM[PG][INDEX]);
                logMsg(stdout, BUFF, 7);
            }

            ind_counter++;
            byte_counter++;
            ADDR_TEMP++;
        }
        else if (ind_counter == BLOCK_WIDTH) {
            // Pad end of line with line # and color
            // Make sure page+index updated in case page transition occurred
            MEM_LOC = ADDR_SPLIT(ADDR_TEMP - 8);
            PG = MEM_LOC[0];
            INDEX = MEM_LOC[1];

            MSG = "\t \033[4m%03d\033[0m\033[45;1m, \033[4m%03d\033[0m\033[45;1m ";
            snprintf(BUFF, 256, MSG, PG, INDEX);
            logMsg(stdout, BUFF, 7);

            LINE_NUM++;
            ind_counter = 0;
        }
        else { 
            if (ADDR_TEMP == ADDR) {
                MSG = "{%02X}";
                snprintf(BUFF, 256, MSG, MEMORY->RAM[PG][INDEX]);
                logMsg(stdout, BUFF, 7);
            }
            else {
                MSG = " %02X ";
                snprintf(BUFF, 256, MSG, MEMORY->RAM[PG][INDEX]);
                logMsg(stdout, BUFF, 7);
            }

            ind_counter++;
            byte_counter++;
            ADDR_TEMP++;
        }
    }

    // Pad end of LAST line with line # and color
    MSG = "\t \033[4m%03d\033[0m\033[45;1m, \033[4m%03d\033[0m\033[45;1m \n";
    snprintf(BUFF, 256, MSG, PG, INDEX - BLOCK_WIDTH + 1);
    logMsg(stdout, BUFF, 7);
    
    return NULL;
}

HBYTE MEM_READ(MEM* MEMORY, ADDRESS ADDR) {
    HBYTE* MEM_LOCATION = ADDR_SPLIT(ADDR);

    return MEMORY->RAM[MEM_LOCATION[0]][MEM_LOCATION[1]];
}

ADDRESS MEM_WRITE(MEM* MEMORY, ADDRESS ADDR, HBYTE DATA) {
    HBYTE* MEM_LOCATION = ADDR_SPLIT(ADDR);

    MEMORY->RAM[MEM_LOCATION[0]][MEM_LOCATION[1]] = DATA;
    return ADDR;
}

HBYTE MEM_WRITE_BULK(MEM* MEMORY, ADDRESS ADDR, HBYTE* DATA, int WIDTH) {
    uint8_t BYTES_WRITTEN = 1;
    uint16_t TOP_INDEX;
    uint16_t CURR_ADDR;
    int TOP_ADDR;

    HBYTE* MEM_LOC = ADDR_SPLIT(ADDR);
    HBYTE PG = MEM_LOC[0];
    HBYTE INDEX = MEM_LOC[1];

    // Check to make sure there is enough space to write the entire thing
    // If the memory index associated w ADDR + (width of data to be written)
    // is <256, then the data will be written on a single page.
    //
    // If that sum exceeds 256, then it will need to be partially split.
    //
    // If that sum causes the written data to exceed the allowed boundary of $FFF9,
    // then the write will fail and NULL will be returned.

    TOP_INDEX = MEM_LOC[1] + WIDTH;
    TOP_ADDR = ADDR + WIDTH;

    // Not enough space to write without bumping into reserved memory
    if (TOP_ADDR > 0xFFF9) {
        printf("WRITE ERROR: Not enough space to write requested %d bytes at $%04d\n", WIDTH, ADDR);
        return 0;
    }

    // Room to write successfully
    // Single page
    if (TOP_INDEX < 256) {
        int n;
        CURR_ADDR = ADDR;

        // Write each HBYTE one at a time from DATA
        for (n = 0; n < WIDTH; n++) {
            MEM_LOC = ADDR_SPLIT(CURR_ADDR);
            PG = MEM_LOC[0];
            INDEX = MEM_LOC[1];

            MEMORY->RAM[PG][INDEX] = DATA[n];

            CURR_ADDR++;
        }
    }

    // Two pages
    else {
        // Free bytes until the end of the page
        uint8_t BYTES_PAGE1 = 256 - MEM_LOC[0];
        // Bytes that will be written to the second page
        uint8_t BYTES_PAGE2 = WIDTH - BYTES_PAGE1;
        printf("Bytes on page %d: %d\nBytes on page %d: %d\n",
            PG, BYTES_PAGE1, (PG+1), BYTES_PAGE2);
    }   
    

    return BYTES_WRITTEN;
}

HBYTE MEM_INIT(MEM* SYSMEM, cJSON* MEM_CONFIG) {
    // Initializes memory contents, including:
    //      -6 bytes system memory ($FFFA-FFFF)
    //          NMI, RST, and IRQ vectors
    //      -Reset code

    ADDRESS IRQ_ADDR;
    ADDRESS RST_ADDR;
    ADDRESS NMI_ADDR;

    HBYTE* IRQ_SPLIT;
    HBYTE* RST_SPLIT;
    HBYTE* NMI_SPLIT;
    
    // Extract interrupt vectors from cJSON object
    cJSON *VECTOR = cJSON_GetObjectItem(MEM_CONFIG, "NMI");
    NMI_ADDR = strtol(VECTOR->valuestring, NULL, 16);
    NMI_SPLIT = ADDR_SPLIT(NMI_ADDR);

    VECTOR = cJSON_GetObjectItem(MEM_CONFIG, "RES");
    RST_ADDR = strtol(VECTOR->valuestring, NULL, 16);
    RST_SPLIT = ADDR_SPLIT(RST_ADDR);

    VECTOR = cJSON_GetObjectItem(MEM_CONFIG, "BRK");
    IRQ_ADDR = strtol(VECTOR->valuestring, NULL, 16);
    IRQ_SPLIT = ADDR_SPLIT(IRQ_ADDR);
    
    // Load vectors into appropriate memory
    // IRQ ADDRESS HIGH BYTE
    MEM_WRITE(SYSMEM, 0xFFFB, IRQ_SPLIT[0]);
    // IRQ ADDRESS LOW BYTE
    MEM_WRITE(SYSMEM, 0xFFFA, IRQ_SPLIT[1]);

    // RESET ADDRESS HIGH BYTE
    MEM_WRITE(SYSMEM, 0xFFFD, RST_SPLIT[0]);
    // RESET ADDRESS LOW BYTE
    MEM_WRITE(SYSMEM, 0xFFFC, RST_SPLIT[1]);

    // NMI ADDRESS HIGH BYTE
    MEM_WRITE(SYSMEM, 0xFFFF, NMI_SPLIT[0]);
    // NMI ADDRESS LOW BYTE
    MEM_WRITE(SYSMEM, 0xFFFE, NMI_SPLIT[1]);
    
    return 0;
}

void STACK_PUSH(MEM* SYSMEM, CPU* SYS, HBYTE DATA, ADDRESS* WRITTEN) {
    // If stack is full, replace byte at top of stack
    if (SYS->SP == 0) {
        MEM_WRITE(SYSMEM, 0x100, DATA);
        if (WRITTEN != NULL) {
            *WRITTEN = 0x100;
        }
    }

    // Otherwise push new byte to top of stack and update STACK_TOP
    else {
        MEM_WRITE(SYSMEM, SYS->SP + 0x100, DATA);
        if (WRITTEN != NULL) {
            *WRITTEN = SYS->SP;
        }
        SYS->SP -= 1;
    }
}

HBYTE STACK_POP(MEM* SYSMEM, CPU* SYS, ADDRESS* WRITTEN) {
    // Stack is empty
    if (SYS->SP == 0xFF) {
        return SYSMEM->RAM[1][255];
    }
    else {
        SYS->SP += 1;
        *WRITTEN = SYS->SP;
        return MEM_READ(SYSMEM, SYS->SP + 0x100);
    }
    
}

ADDRESS ADDR_CONCAT(HBYTE* DATA) {
    ADDRESS CONC;
    CONC = (DATA[0] * 256) + DATA[1];
    return CONC;
}

HBYTE* ADDR_SPLIT(ADDRESS DATA) {
    HBYTE *SPLIT = malloc(sizeof(HBYTE) * 2);

    SPLIT[0] = (HBYTE)(DATA / 256);
    SPLIT[1] = (HBYTE)DATA - (256 * SPLIT[0]);

    return SPLIT;
}