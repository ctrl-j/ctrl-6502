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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include "../INCLUDES/cJSON.h"
#include <raylib.h>

// User-defined types for the system and CPU
typedef uint8_t BYTE;
// 2 Bytes
typedef uint16_t WORD;
// 256 B memory
typedef BYTE PAGE[256];
// 32KB memory
typedef PAGE BANK[128];

typedef enum ScreenPhase {UNKNOWN = -1, INIT = 0, MAIN} ScreenPhase;

typedef struct _CPU {
    WORD BUS_ADDRESS;   // Address bus
    BYTE BUS_DATA;      // Data bus

    WORD PC;    // Program counter    
    BYTE SP;    // Stack pointer   
    BYTE A;     // Accumulator
    BYTE X;     // X index register
    BYTE Y;     // Y index register
    BYTE S;     // Status register

    bool WRITE_ENABLE;

    //////////////////////
    // CPU Status Register
    // ///////////////////
    // 76543210
    // NV1BDIZC
    //////////////////
    bool NEGATIVE;
    bool OVERFLOW;

    bool BREAK;
    bool DECIMAL;
    bool INTERRUPT_DISABLE;
    bool ZERO;
    bool CARRY;

    // Detection of an NMI or IRQ hardware interrupt, 
    // or BRK instruction, all execute the same process on 6502:
    //      1. CPU finishes current instruction and associated processing
    //      2. MSB of Program Counter (PC) pushed onto Stack (1B)
    //      3. LSB of PC pushed onto Stack (1B)
    //      4. Status Register (SR) pushed onto stack (1B)
    //      5. Interrupt Disable CPU flag is set to 1 (b2 in SR)
    //      6. PC is loaded from relevant vector, depending on interrupt type
    //          Type          [LSB]  [MSB]
    //            IRQ/BRK:    $FFFE, $FFFF
    //            RST:        $FFFC, $FFFD
    //            NMI:        $FFFA, $FFFB

    bool NMI;
    bool IRQ;
    bool RST;

} CPU;

typedef struct _MEMORY {
    BANK RAM;
    BANK ROM;

} MEMORY;

typedef struct _INSTRUCTION {
    char* MNEM;
    // Hex value of associated machine code
    BYTE VAL;

    BYTE ADDR_MODE;

    int (*FUNC)(BYTE, WORD);
} INSTRUCTION;

/*
// User-defined struct to hold all important information
// about the graphics of a screen and also child screens
// Allows you to update properties of a screen easily, as the contents of that screen's
// APP_SCREEN struct will be pulled from every frame update
typedef struct _APP_SCREEN {
    int ID;
    int WIDTH;
    int HEIGHT;
    Color BG_COLOR;
    Color TEXT_COLOR;
} APP_SCREEN;
*/

extern ScreenPhase currentScreen;
extern Font mainFont;
//extern Music mainAudio;
extern Sound mainFX;

#endif