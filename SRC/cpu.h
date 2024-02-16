/*
    Copyright (C) 2024  Joshua Jackson
    [Full GPL-3.0 license can be found in LICENSE, in the root folder]
*/

#ifndef __CPU_H__
#define __CPU_H__

#include "./INCLUDES/cJSON.h"
#include <raylib.h>

/********************************************
* RAYLIB Types and Structures definitions
********************************************/
typedef enum ScreenPhase {UNKNOWN = -1, INIT = 0, MAIN} ScreenPhase;

/********************************************
* RAYLIB Global variables
********************************************/
extern ScreenPhase currentScreen;
extern Font font;
extern Music audio;
extern Sound soundfx;

// Contains 2 hex digits
typedef uint8_t HBYTE;

// 16-bit address
typedef uint16_t ADDRESS;

// Array of 256 hex digits = 256 bytes per page
typedef HBYTE PAGE[256];

typedef PAGE BANK[128];

typedef struct _CPU {
    //////////////////
    // CPU Buses
    //////////////////
    // System Address Bus
    ADDRESS ADDR_BUS;

    // System Data Bus (Bi-directional, determined by Write-enable)
    HBYTE DATA_BUS;
    
    ////////////////////////////
    // Internal Registers
    ////////////////////////////
    // Program Counter
    ADDRESS PC;

    // Stack Pointer
    HBYTE SP;
    // Accumulator
    HBYTE ACC;
    // Index Register X
    HBYTE X;
    // Index Register Y
    HBYTE Y;
    // CPU Status
    HBYTE STATUS;

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

    // R/W Line
    // "READ" mode when 1, "WRITE" mode when 0
    bool WRITE_ENABLE;

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
    bool BRK;

    bool RST;

} CPU;

//Mem accessed by address.
//Mem zone identified and then switch case to set offset and then index
//directly into appropriate container.
typedef struct _MEM {

    // 65,536 addresses total - 2x 256-byte pages - 6 bytes =
    // (254x 256-byte pages) - 6 bytes = 
    // 253x 256-byte pages + 250/256 in extra page
    PAGE RAM[256];

    ///////////////////////////////////////////
    // 6 bytes of reserved mem $FFFA - $FFFF //
    ////////////////////////////////////////////////////
    //
    // Non-Maskable Interrupt (NMI)        : $FFFA/B
    //      :
    // Power-On Reset (PWR_ON_RST)         : $FFFC/D
    //      :
    // Break / Interrupt Request (BRK_IRQ) : $FFFE/F
    //      :
    //
    ////////////////////////////////////////////////////

    /////////////////////////////////////////////////////
    // Two pages of reserved memory, starting at $0000 //
    /////////////////////////////////////////////////////
    // Zero Page (ZP) : $0000 - $00FF 
    //      : for special addressing modes
    // Stack (STACK)  
    //      : $0100 - $01FF : system stack memory
    ///////////////////////////////////////////////

    /** Memory Map - 64 KB addressable memory
     * First 32 KB addressed to RAM
     * Second 32 KB addressed to ROM, can be bank switched to go through more data
     * 
     * 32 KB (32,768 B) = 256 Kbit (262,144 bit) = Addresses $0000 - $7FFF 
     * Size of 28C256 Paged Parallel EEPROM, and 62256 Static RAM
     * 
     * RAM:
     *      $0000 - $00FF    Zero Page (ZP)      [RESERVED]
     *      $0100 - $01FF    Stack               [RESERVED]
     * 
     * ROM:
     *  
     *      $7FFF - $FFF9    Program ROM         [If program has graphics or audio data {specified by
     *                                            file and page headers} it is read before rest of 
     *                                            data, and then loaded into RAM]
     *      $FFFA - $FFFF    System vectors      [RESERVED]
     * 
     **/ 

} MEM;

typedef struct _PRG_ROM {
    // Structure to hold temporary binary data being read from code file
    // If headers indicate graphics or audio data, that stuff is transferred
    //      from this struct into system RAM in appropriate locations.
    // The rest of the data is mapped to the upper 32KB ROM section of system memory.
    // If file being loaded is > 32KB, it is loaded in "chunks" or 32 KB banks.

    // When bin file loaded, entire contents are split into 32KB chunks (or "BANK"s)
    // and stored as array of BANKs in PRG_DATA.
    BANK* PRG_DATA;

    // Total number of used banks in the source file
    HBYTE TOTAL_BANKS;
    // Bank currently being accessed by the CPU
    HBYTE CURR_BANK;


} PRG_ROM;

typedef struct _INSTR {
    char* MNEM;
    
    // 0 - impl     (implied)               OPC
    // 1 - A        (accumulator)           OPC A
    // 2 - #        (immediate)             OPC #$BB
    // 3 - zpg      (zero page)             OPC $LL
    // 4 - zpg,X    (zero page, X-indexed)  OPC $LL,X
    // 5 - zpg,Y    (zero page, Y-indexed)  OPC $LL,Y
    // 6 - rel      (relative)              OPC $BB
    // 7 - abs      (absolute)              OPC $LLHH
    // 8 - abs,X    (absolute, X-indexed)   OPC $LLHH,X
    // 9 - abs,Y    (absolute, Y-indexed)   OPC $LLHH,Y
    // 10- ind      (indirect)              OPC ($LLHH)
    // 11- X,ind    (X-indexed, indirect)   OPC ($LL,X)
    // 12- ind,Y    (indirect, Y-indexed)   OPC ($LL),Y
    HBYTE A_MODE;

    // Hex value of associated machine code
    HBYTE VAL;

    int (*FUNC)(MEM*, CPU*, HBYTE, ADDRESS);

} INSTR;

#include "mem.h"

// Initializes CPU state machine and instruction set
HBYTE CPU_INIT(INSTR** INSTR_SET, MEM* SYSMEM, CPU* SYS);

// Main operation loop for the CPU
HBYTE CPU_MAIN(MEM* SYSMEM, CPU* SYS, INSTR** INSTR_SET);

// CPU Interrupt Service Routine (ISR)
ADDRESS CPU_ISR_HANDLER(MEM* SYSMEM, CPU* SYS);

// Sets FLAG to VAL, both in SYS struct flags and numeric/binary value
// Returns previous value of FLAG
bool CPU_SET_STATUS(CPU* SYS, HBYTE FLAG, bool VAL);

// Returns value of bit# FLAG in CPU struct status register
bool CPU_GET_STATUS(CPU* SYS, HBYTE FLAG);

HBYTE INSTR_FETCH_AND_EXECUTE(INSTR** INSTR_SET, MEM* SYSMEM, CPU* SYS);

// Decodes and executes INSTRUCTION
int INSTR_DECODE(INSTR** INSTR_SET, MEM* SYSMEM, CPU* SYS, HBYTE* OPCODE, ADDRESS *DATA);

int ADC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int AND(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int ASL(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BCC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BCS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BEQ(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BIT(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BMI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BNE(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BPL(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BRK(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BVC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int BVS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int CLC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int CLD(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int CLI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int CLV(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int CMP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int CPX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int CPY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int DEC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int DEX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int DEY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int EOR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int INC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int INX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int INY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int JMP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int JSR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int LDA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int LDX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int LDY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int LSR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int NOP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int ORA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int PHA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int PHP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int PLA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int PLP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int ROL(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int ROR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int RTI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int RTS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int SBC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int SEC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int SED(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int SEI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int STA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int STX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int STY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int TAX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int TAY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int TSX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int TXA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int TXS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);
int TYA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND);

#endif