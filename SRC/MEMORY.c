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
#include <stdio.h>
#include <stdlib.h>

#include "MEMORY.h"
#include "SYSTEM.h"

void MEM_LOAD(MEMORY* MEMORY, char* SRC,
                   WORD SRC_START, WORD SRC_WIDTH,
                   WORD MEM_START, bool PRESERVE) {
    WORD WRITE = MEM_START;
    WORD SRC_COUNT = SRC_START;
    WORD WIDTH = SRC_WIDTH;
    BYTE BYTE = 0;
    int COUNT = 0;


    if (MEMORY == NULL) {
        logMsg(stdout, "*** MEM_LOAD: MEMORY* MEMORY is NULL ***\n", 9);
        return;
    }

    FILE* CODE = fopen(SRC, "rb");
    if (CODE == NULL) {
        logMsg(stdout, "*** MEM_LOAD: Unable to open SRC ***\n", 9);
    }

    // If SRC_WIDTH set to 0, then use entire file
    if (SRC_WIDTH == 0) {
        while (!feof(CODE)) {
            fgetc(CODE);
            COUNT++;
        }
        WIDTH = COUNT - 1;
    }

    fseek(CODE, SRC_START, SEEK_SET);

    for (SRC_COUNT = SRC_START; SRC_COUNT < (SRC_START + WIDTH); SRC_COUNT++) {
        if (feof(CODE)) {

        }
        else {
            BYTE = fgetc(CODE);
            MEM_WRITE(MEMORY, WRITE, BYTE);
            WRITE++;
        }
    }
}

void MEM_SAVE(MEMORY* MEMORY, char* DST, WORD MEM_START, WORD MEM_WIDTH) {
    WORD WIDTH = MEM_WIDTH;
    WORD INC;
    BYTE BYTE;
    
    if (MEMORY == NULL) {
        logMsg(stdout, "*** MEM_TO_BIN: MEMORY* MEMORY is NULL ***\n", 9);
        return;
    }

    FILE* BIN = fopen(DST, "wb");
    if (BIN == NULL) {
        logMsg(stdout, "*** MEM_TO_BIN: Unable to create DST binary file ***\n", 9);
    }

    if (MEM_WIDTH == 0) {
        WIDTH = 0xFFFF - MEM_START;
    }
    
    for (INC = MEM_START; INC < (MEM_START + WIDTH); INC++) {
        BYTE = MEM_READ(MEMORY, INC);
        
        fputc(BYTE, BIN);
    }
    fclose(BIN);

}