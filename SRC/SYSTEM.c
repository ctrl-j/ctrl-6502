/*
    ctrl-6502: a C-based emulator of an MOS 6502 system
    Copyright (C) 2024  Joshua Jackson

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// [Full GPL-3.0 license can be found in LICENSE, in the root folder]

///////////////////////////////////////////////////////////////////////////
// SYSTEM.c: System Monitor and "Virtual Firmware Integration"
//      Contains main function, operations init for everything
//      Calls main loop, interacts b/t CPU and virtual peripherals (IO, display, serial, etc)
//////////////////////////////////////////////////////////////////////////

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>

// External libraries

// raylib is an external library for audio, graphics, other I/O primarily focused on making games
// ***credit/citation needed***
#include <raylib.h>

// cJSON is written by Dave Gamble and other contributers under the MIT License
// ***citation needed***
#include "../INCLUDES/cJSON.h"

// Local includes
#include "SYSTEM.h"
#include "CPU.h"
#include "screens.h"

///////////////////////////////

ScreenPhase currentScreen = INIT;
Font font = { 0 };
Music audio = { 0 };
Sound soundfx = { 0 };

// "GlObAl" structures
CPU* SYSTEM_CORE;
MEMORY* SYSTEM_MEMORY;

int MAIN_LOOP();

int main(int argc, char **argv) {
    int i = 0;

    char BUFF[256];
    char *MSG;

    // Get platform from executable name (WINDOWS, LINUX, or DEBUG)
    char* EXEC_NAME = basename(argv[0]);
    char *PLATFORM = NULL;

    printf("%s\n", EXEC_NAME);
    if (EXEC_NAME[0] == 'w') {
        PLATFORM = "WINDOWS";
    }
    else if (EXEC_NAME[0] == 'l') {
        PLATFORM = "LINUX";
    }
    else if (EXEC_NAME[0] == 'd') {
        PLATFORM = "DEBUG";
    }
    else {
        PLATFORM = "UNDEFINED";
    }

    ////////////////////////////////////////////
    // STEP 0:
    // Allocate memory for system structures,
    // Read + parse JSON config 
    ////////////////////////////////////////////
    // Useful config fields:
    //      -CPU Startup action: "BIOS" or "[PROGRAM].mc"
    //          Either loads default ctrl65-OS or user-program denoted by [PROGRAM].mc
    //          BIOS expected to be "BIOS.mc" in SYS directory
    //          User-programs should be put into PROG directory
    //      -Startup mode: "TEXT" or "GRAPHICS"
    //          Self explanatory. May make bios function to switch between the two
    //      -Resolution:
    //          ***Insert default res vals***
    //          1800 x 1200 ???
    // MEMORY CONFIG
    ////////////////
    //      -RAM_SIZE : size of R/W RAM in bytes
    //      -ROM_SIZE: " " " ROM in bytes
    //      -SYS_VAR_LOCATION: [start of location offset,
    //                          size of variable section]
    ////////////////////////////////////////////
    // Allocate memory for CPU structure, system memory, instruction set
    /////////////////////////////////////////////////////
    SYSTEM_CORE = malloc(sizeof(CPU));
    SYSTEM_MEMORY = malloc(sizeof(MEMORY));
    
    INSTRUCTION* INSTR_SET[256];
    for (i = 0; i < 256; i++) {
        INSTR_SET[i] = malloc(sizeof(INSTRUCTION));
        if (INSTR_SET[i] == NULL) {
            printf("CPU INIT ERROR: Failed to allocate memory while building instruction set.\n");
            return EXIT_FAILURE;
        }
    }
    ///////////////////////////
    // Open configuartion file
    ///////////////////////////
    FILE *CONFIG = fopen("../SYS/config.json", "r");
    if (CONFIG == NULL) {
        logMsg(stdout, "*** CANNOT OPEN CONFIG.JSON, TERMINATING ***\n", 9);
        return EXIT_FAILURE;
    }

    // Read file contents into buffer and close config.json
    char JSON_BUFF[2048];
    fread(JSON_BUFF, 1, sizeof(JSON_BUFF), CONFIG);
    fclose(CONFIG);

    // Parse JSON data
    cJSON *JSON = cJSON_Parse(JSON_BUFF);
    if (JSON == NULL) {
        const char *ERR_PTR = cJSON_GetErrorPtr();
        if (ERR_PTR != NULL) {
            MSG = "*** ERROR PARSING CONFIG.JSON ***\n";
            printf(MSG);
        }

        cJSON_Delete(JSON);
        return EXIT_FAILURE;
    }

    ////////////////////////////////////////////
    // STEP 1:
    // ROM initialization
    // Load BIOS code or program into system memory
    // Includes reset vector pointing to ROM
    ////////////////////////////////////////////

    ////////////////////////////////////////////
    // STEP 2:
    // CPU Initialization
    // Init the structure and fields
    ////////////////////////////////////////////

    ////////////////////////////////////////////
    // STEP 3:
    // Initialize raylib window,
    // other graphics config information
    ////////////////////////////////////////////

    ////////////////////////////////////////////
    // STEP 4:
    // Start main CPU loop
    ////////////////////////////////////////////

    return EXIT_SUCCESS;
}

int MAIN_LOOP() {
    // Raylib draw loop

    // CPU Loop:
    //      1. Fetch instruction from PC location
    //      2. Execute instruction
    //          A. Determine addressing mode
    //             Addressing mode-specific data fetch operations:
    //              Accumulator (2 cycles) - reads current value of accumulator register
    //              Immediate (2 cycles) - 1 byte read following instr
    //              Implied (2 cycles) - no data, for BRK and flag instructions
    //              Relative (2/3 cycles) - 1 signed byte read following instr,
    //                                      offsets +/-127 from current PC. Extra cycle used if offset causes a cross into a new page
    //              Zero Page (3 cycles) - 1 byte offset from the zero page
    //              Zero Page X- or Y- indexed (4 cycles) - adds index to 1 byte offset; If index addition causes carry, 
    //                                                      the carry is dropped and returned address wraps around to 0
    //              Absolute, Absolute X- and Y-indexed (4/5 cycles) - Word/address read following instr;
    //                              Can also be X/Y indexed. Extra cycle used if crossing a page when using index
    //              Indirect (5 cycles) - Word read as memory pointer following 
    //                                  instr, then reads value stored at that pointer
    //              Indirect X- and Y-indexed (5/6 cycles for Y, 6 cycles for X) -
    //                                1 byte read following instr, added to ZP, indexed by X or Y;
    //                                Word located at the calculated address is returned;
    //                                X-index increments w/o carry, Y-index increments w/ carry (extra cycle if carry)
    //          B. Set CPU state machine input
    //              Nothing: (Acc, Implied)
    //              Byte: (Indirect X/Y, ZP, ZP X/Y, Immediate, Relative)
    //              Word: (Absolute, Absolute X/Y, Indirect)
    //              
    //              Post:
    //                  X or Y indexed?
    //                      Swap index reg to X/Y if not eq. to instr addr mode
    //                  Indirect?
    //                      Grab word at location pointed to by data
    //          C. Perform instruction function using appropriate input
    //      3. Interrupt Handling
    //          Handled after the instr fetch+execution since 6502 allows instr to finish
    //          when interrupt is called anyways. IDK how accurate this is to the actual order it occurs
    //          -RST:
    //              Reset routine, only called on startup or system reset
    //          -BRK/IRQ:
    //              Can be initiated via the BRK instruction or using IRQ pin (emulated)
    //              Can be masked/disabled with the interrupt disable CPU status flag.
    //          -NMI:   
    //              Non-maskable interrupt, initiated using NMI pin (emulated)
    //////////////////////////////////////////////
    // When interrupt request is called,
    //      1. Pushes HIGH BYTE of return address to stack
    //      2. Pushes LOW BYTE of return address to stack
    //      3. Pushes status register to stack
    //      4. Get IRQ/NMI vector LOW BYTE from $FFFE/A
    //      5. Get IRQ/NMI vector HIGH BYTE from $FFFF/B
    //      6. Jump to vector and begin executing ISR         

}