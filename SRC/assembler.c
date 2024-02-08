/*
    Copyright (C) 2024  Joshua Jackson
    [Full GPL-3.0 license can be found in LICENSE, in the root folder]
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "INCLUDES/cJSON.h"
#include "assembler.h"
#include "cpu.h"
#include "mem.h"
#include "util.h"

void TXT_TO_BIN(char* TXT_FILENAME, char* BIN_FILENAME) {
    char BUFF[256];
    char *MSG;

    FILE* SRC = fopen(TXT_FILENAME, "r");
    if (SRC == NULL) {
        MSG = "*** TXT_TO_BIN: Unable to open source file %s ***\n";
        snprintf(BUFF, 256, MSG, TXT_FILENAME);
        logMsg(stdout, BUFF, 9);
    }

}

void MEM_LOAD(MEM* MEMORY, char* SRC,
                   ADDRESS SRC_START, ADDRESS SRC_WIDTH,
                   ADDRESS MEM_START, bool PRESERVE) {
    ADDRESS WRITE = MEM_START;
    ADDRESS SRC_COUNT = SRC_START;
    ADDRESS WIDTH = SRC_WIDTH;
    HBYTE BYTE = 0;
    int COUNT = 0;


    if (MEMORY == NULL) {
        logMsg(stdout, "*** MEM_LOAD: MEM* MEMORY is NULL ***\n", 9);
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

void MEM_TO_BIN(MEM* MEMORY, char* DST, ADDRESS MEM_START, ADDRESS MEM_WIDTH) {
    ADDRESS WIDTH = MEM_WIDTH;
    ADDRESS INC;
    HBYTE BYTE;
    
    if (MEMORY == NULL) {
        logMsg(stdout, "*** MEM_TO_BIN: MEM* MEMORY is NULL ***\n", 9);
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

INSTR* PARSE(FILE* CODE) {

    return NULL;
}