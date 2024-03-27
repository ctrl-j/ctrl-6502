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

int main(int argc, char **argv) {
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
    CPU* CPU_MAIN = malloc(sizeof(CPU));
    MEMORY* MEMORY_MAIN = malloc(sizeof(MEMORY));
    
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
            snprintf(BUFF, 256, MSG, ERR_PTR);
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