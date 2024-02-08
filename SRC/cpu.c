#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <raylib.h>
#include <string.h>
#include <libgen.h>

// cJSON is written by Dave Gamble and other contributers under the MIT License
#include "./INCLUDES/cJSON.h"

#include "cpu.h"
#include "mem.h"
#include "assembler.h"
#include "tests.h"
#include "util.h"
#include "graphics.h"

ScreenPhase currentScreen = INIT;
Font font = { 0 };
Music audio = { 0 };
Sound soundfx = { 0 };

// ARGV[1]: INIT_SOURCE
//      0- binary file with all init code
//          Loaded to SYSMEM: reset vectors, BIOS code
//          Loaded to VMEM:   colors, palettes, 2x nametables,
//                            
//      1- 

int main(int argc, char **argv){
    HBYTE INIT_STATUS;
    HBYTE MAIN_STATUS;
    HBYTE MEM_STATUS;
    HBYTE GRAPHICS_STATUS;

    char BUFF[256];
    char *ERR;
    char *MSG;
    char *PLATFORM;
    
    int i;

    //cJSON *MEM_MAP = NULL;
    cJSON *SYSTEM_VECTORS = NULL;
    cJSON *GRAPHICS_CONFIG = NULL;

    // Get platform from executable name (WINDOWS, LINUX, or DEBUG)
    char* EXEC_NAME = basename(argv[0]);

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
    // Allocate memory for system structures:
    //
    ////////////////////////////////////////////
    // Allocate memory for CPU* SYS structure
    ////////////////////////////////////////////
    CPU* SYS = malloc(sizeof(CPU));
    ////////////////////////////////////////////
    // Allocate memory for MEM* SYSMEM structure
    ////////////////////////////////////////////
    MEM* SYSMEM = malloc(sizeof(MEM));
    ////////////////////////////////////////////
    // Allocate memory for instruction set array
    ////////////////////////////////////////////
    INSTR* INSTR_SET[256];
    for (i = 0; i < 256; i++) {
        INSTR_SET[i] = malloc(sizeof(INSTR));
        if (INSTR_SET[i] == NULL) {
            printf("CPU INIT ERROR: Failed to allocate memory while building instruction set.\n");
            return EXIT_FAILURE;
        }
    }

    if (argc == 2 && strtol(argv[1], NULL, 10) == 1) {
        //////////////////////////
        // Read + parse JSON config file before init
        ////////////////////////////////////////////
        FILE *CONFIG = fopen("SYS/config.json", "r");
        if (CONFIG == NULL) {
            logMsg(stdout, "*** CANNOT OPEN CONFIG.JSON, TERMINATING ***\n", 9);
            return EXIT_FAILURE;
        }

        // Read file contents into buffer and close config.json
        char JSON_BUFF[1024];
        fread(JSON_BUFF, 1, sizeof(JSON_BUFF), CONFIG);
        fclose(CONFIG);

        // Parse JSON data
        cJSON *JSON = cJSON_Parse(JSON_BUFF);
        if (JSON == NULL) {
            const char *ERR_PTR = cJSON_GetErrorPtr();
            if (ERR_PTR != NULL) {
                MSG = "*** ERROR PARSING CONFIG.JSON: %s ***\n";
                snprintf(BUFF, 256, MSG, ERR_PTR);
                logMsg(stdout, BUFF, 9);
            }

            cJSON_Delete(JSON);
            return EXIT_FAILURE;
        }

        // Access JSON data
        //MEM_MAP = cJSON_GetObjectItem(JSON, "MEM_MAP");

        SYSTEM_VECTORS = cJSON_GetObjectItemCaseSensitive(JSON, "MEM_CONFIG");

        GRAPHICS_CONFIG = cJSON_GetObjectItem(JSON, "GRAPHICS_CONFIG");
    }

    /////////////////////////
    // STEP 1:
    // Memory initialization
    //////////////////////////
    
    //TODO:
    //  -write reset vector to $FFFC/FFFD
    //  -write BIOS code to appropriate location
    MEM_STATUS = MEM_INIT(SYSMEM, SYSTEM_VECTORS);
    if (MEM_STATUS != 0) {
        ERR = "*** SYSTEM ERROR *** MEMORY INITIALIZATION FAILED *** STATUS: %d ***\n\n";
        
        snprintf(BUFF, 256, ERR, MEM_STATUS);
        logMsg(stdout, BUFF, 9);
        return EXIT_FAILURE;
    }
    ///////////////////////
    // STEP 2:
    // CPU Initialization
    ///////////////////////
    INIT_STATUS = CPU_INIT(INSTR_SET, SYSMEM, SYS);

    // CPU init unsuccessful
    if (INIT_STATUS != 0) {
        ERR = "*** SYSTEM ERROR *** CPU INITIALIZATION FAILED *** STATUS: %d ***\n\n";
        
        snprintf(BUFF, 256, ERR, INIT_STATUS);
        logMsg(stdout, BUFF, 9);
        return EXIT_FAILURE;
    }

    /////////////////////////////////////
    // STEP 3:
    // Initialize raylib window,
    // other graphics config information
    /////////////////////////////////////
    GRAPHICS_STATUS = GRAPHICS_INIT(GRAPHICS_CONFIG, PLATFORM);
    // CPU init unsuccessful
    if (GRAPHICS_STATUS != 0) {
        ERR = "*** SYSTEM ERROR *** GRAPHICS INITIALIZATION FAILED *** STATUS: %d ***\n\n";
        
        snprintf(BUFF, 256, ERR, GRAPHICS_STATUS);
        logMsg(stdout, BUFF, 9);
        return EXIT_FAILURE;
    }

    ////////////////////////
    // STEP 4:
    // Start main CPU loop
    ////////////////////////
    MAIN_STATUS = CPU_MAIN(SYSMEM, SYS, INSTR_SET);

    MSG = "\n*** CPU Stopped. Exit status: %02x ***\n\n";

    snprintf(BUFF, 256, MSG, MAIN_STATUS);
    logMsg(stdout, BUFF, 8);

    return EXIT_SUCCESS;
}

HBYTE CPU_INIT(INSTR** INSTR_SET, MEM* SYSMEM, CPU* SYS) {
    HBYTE i;
    int OP_COUNT;

    INSTR* NEXT_INSTR;
    //char BUFF[256];
    //char *MSG;

    //////////////////////////
    // How a baby instruction set is born
    // - The Miracle of Life, by ctrlj
    //////////////////////////
    
    //////////////////////////
    // Step 1: Initialize instruction set array. 
    // Index # corresponds to opcode of INSTR stored at that index.
    // Allows for easy access to instruction set info with one value.    
    char* MNEMONICS[] = {
            "BRK", "ORA", "ORA", "ASL", "PHP", "ORA", "ASL", "ORA", "ASL",
            "BPL", "ORA", "ORA", "ASL", "CLC", "ORA", "ORA", "ASL", 
            "JSR", "AND", "BIT", "AND", "ROL", "PLP", "AND", "ROL", "BIT", "AND", "ROL", 
            "BMI", "AND", "AND", "ROL", "SEC", "AND", "AND", "ROL",
            "RTI", "EOR", "EOR", "LSR", "PHA", "EOR", "LSR", "JMP", "EOR", "LSR", 
            "BVC", "EOR", "EOR", "LSR", "CLI", "EOR", "EOR", "LSR", 
            "RTS", "ADC", "ADC", "ROR", "PLA", "ADC", "ROR", "JMP", "ADC", "ROR",
            "BVS", "ADC", "ADC", "ROR", "SEI", "ADC", "ADC", "ROR",
            "STA", "STY", "STA", "STX", "DEY", "TXA", "STY", "STA", "STX", 
            "BCC", "STA", "STY", "STA", "STX", "TYA", "STA", "TXS", "STA", 
            "LDY", "LDA", "LDX", "LDY", "LDA", "LDX", "TAY", "LDA", "TAX", "LDY", "LDA", "LDX",
            "BCS", "LDA", "LDY", "LDA", "LDX", "CLV", "LDA", "TSX", "LDY", "LDA", "LDX",
            "CPY", "CMP", "CPY", "CMP", "DEC", "INY", "CMP", "DEX", "CPY", "CMP", "DEC", 
            "BNE", "CMP", "CMP", "DEC", "CLD", "CMP", "CMP", "DEC", 
            "CPX", "SBC", "CPX", "SBC", "INC", "INX", "SBC", "NOP", "CPX", "SBC", "INC",
            "BEQ", "SBC", "SBC", "INC", "SED", "SBC", "SBC", "INC"
    };

    HBYTE ADDRESSING_MODES[] = {
        0, 11, 3, 3, 0, 2, 1, 7,
        6, 12, 4, 4, 0, 9, 8, 8,
        7, 11, 3, 3, 3, 0, 2, 1, 7, 7, 7,
        6, 12, 4, 4, 0, 9, 8, 8,
        0, 11, 3, 3, 0, 2, 1, 7, 7, 7,
        6, 12, 4, 4, 0, 9, 8, 8,
        0, 11, 3, 3, 0, 2, 1, 10, 7, 7,
        6, 12, 4, 4, 0, 9, 8, 8,
           11, 3, 3, 3, 0, 0, 7, 7, 7,
        6, 12, 4, 4, 5, 0, 9, 0, 8,
        2, 11, 2, 3, 3, 3, 0, 2, 0, 7, 7, 7,
        6, 12, 4, 4, 4, 5, 0, 9, 0, 8, 8, 9,
        2, 11, 3, 3, 3, 0, 2, 0, 7, 7, 7,
        6, 12, 4, 4, 0, 9, 8, 8,
        2, 11, 3, 3, 3, 0, 2, 0, 7, 7, 7,
        6, 12, 4, 4, 0, 9, 8, 8
    };

    HBYTE OPCODES[] = {
        0,   1,   5,   6,   8,   9,   10,  13,  14, 
        16,  17,  21,  22,  24,  25,  29,  30,
        32,  33,  36,  37,  38,  40,  41,  42,  44,  45,  46,
        48,  49,  53,  54,  56,  57,  61,  62, 
        64,  65,  69,  70,  72,  73,  74,  76,  77,  78,
        80,  81,  85,  86,  88,  89,  93,  94, 
        96,  97,  101, 102, 104, 105, 106, 108, 109, 110,
        112, 113, 117, 118, 120, 121, 125, 126, 
        129, 132, 133, 134, 136, 138, 140, 141, 142, 
        144, 145, 148, 149, 150, 152, 153, 154, 157,
        160, 161, 162, 164, 165, 166, 168, 169, 170, 172, 173, 174,
        176, 177, 180, 181, 182, 184, 185, 186, 188, 189, 190,
        192, 193, 196, 197, 198, 200, 201, 202, 204, 205, 206,
        208, 209, 213, 214, 216, 217, 221, 222, 
        224, 225, 228, 229, 230, 232, 233, 234, 236, 237, 238,
        240, 241, 245, 246, 248, 249, 253, 254
    };

    int (*FUNCTIONS[151])(MEM*, CPU*, HBYTE, ADDRESS) = {
        BRK, ORA, ORA, ASL, PHP, ORA, ASL, ORA, ASL,
        BPL, ORA, ORA, ASL, CLC, ORA, ORA, ASL,
        JSR, AND, BIT, AND, ROL, PLP, AND, ROL, BIT, AND, ROL,
        BMI, AND, AND, ROL, SEC, AND, AND, ROL,
        RTI, EOR, EOR, LSR, PHA, EOR, LSR, JMP, EOR, LSR,
        BVC, EOR, EOR, LSR, CLI, EOR, EOR, LSR, 
        RTS, ADC, ADC, ROR, PLA, ADC, ROR, JMP, ADC, ROR,
        STA, STY, STA, STX, DEY, TXA, STY, STA, STX,
        BCC, STA, STY, STA, STX, TYA, STA, TXS, STA, 
        LDY, LDA, LDX, LDY, LDA, LDX, TAY, LDA, TAX, LDY, LDA, LDX,
        BCS, LDA, LDY, LDA, LDX, CLV, LDA, TSX, LDY, LDA, LDX, 
        CPY, CMP, CPY, CMP, DEC, INY, CMP, DEX, CPY, CMP, DEC, 
        BNE, CMP, CMP, DEC, CLD, CMP, CMP, DEC,
        CPX, SBC, CPX, SBC, INC, INX, SBC, NOP, CPX, SBC, INC,
        BEQ, SBC, SBC, INC, SED, SBC, SBC, INC
    };

    ///////////////////////////////////////// 
    // Step 2: populate the INSTR array with instruction information.
    // 151 unique items for (opcode + mnemonic + addressing mode)
    // Gotta catch 'em all!!!!!!!!
    //
    // INSTR_SET** has 256 spaces, some will be left empty (unused opcodes).
    // Unused indices/opcodes will be set to NULL.
    OP_COUNT = 0;
    for (i = 0; i < 255; i++) {
        if (i == OPCODES[OP_COUNT]) {
            NEXT_INSTR = INSTR_SET[i];

            NEXT_INSTR->MNEM = MNEMONICS[OP_COUNT];
            NEXT_INSTR->A_MODE = ADDRESSING_MODES[OP_COUNT];
            NEXT_INSTR->VAL = OPCODES[OP_COUNT];
            NEXT_INSTR->FUNC = FUNCTIONS[OP_COUNT];
            
            INSTR_SET[i] = NEXT_INSTR;
            OP_COUNT++;
        }
        else {
            free(INSTR_SET[i]);
            INSTR_SET[i] = NULL;
        }
    }

    // Call Power-on-reset routine by setting program counter to reset vector
    SYS->PC      = 0x0;
    SYS->STATUS  = 0x0;
    SYS->SP      = 0xFF;
    CPU_ISR_HANDLER(SYSMEM, SYS);
    
    // Initialize CPU Status
    SYS->NEGATIVE           = false;
    SYS->OVERFLOW           = false;
    SYS->BREAK              = false;
    SYS->DECIMAL            = false;
    SYS->INTERRUPT_DISABLE  = false;
    SYS->ZERO               = false;
    SYS->CARRY              = false;

    // Interrupt status init
    SYS->NMI     = false;
    SYS->RST     = false;
    SYS->BRK     = false;
    SYS->IRQ     = false;

    return 0;
}

HBYTE CPU_MAIN(MEM* SYSMEM, CPU* SYS, INSTR** INSTR_SET) {
    HBYTE MAIN_STATUS = 0;
    
    char BUFF[256];
    char *MSG;

    //bool CPU_RUNNING = true;

    //////////////////////////////////
    // Startup messages
    //////////////////////////////////

        // Clear terminal, set cursor to top
    logMsg(stdout, NULL, -1);
    // Define text color standards
    logMsg(stdout, "System Message\t", 8);
    logMsg(stdout, "General I/O\t", 12);
    logMsg(stdout, "Memory / Data\n", 7);
    logMsg(stdout, "Error Message\t", 9);
    logMsg(stdout, "Success Message\t", 10);
    logMsg(stdout, "Information\n\n", 11);

    logMsg(stdout, "\
                                                                         \n\
 ██████╗████████╗██████╗ ██╗            ██████╗ ███████╗ ██████╗ ██████╗ \n\
██╔════╝╚══██╔══╝██╔══██╗██║           ██╔════╝ ██╔════╝██╔═████╗╚════██╗\n\
██║        ██║   ██████╔╝██║     █████╗███████╗ ███████╗██║██╔██║ █████╔╝\n\
██║        ██║   ██╔══██╗██║     ╚════╝██╔═══██╗╚════██║████╔╝██║██╔═══╝ \n\
╚██████╗   ██║   ██║  ██║███████╗      ╚██████╔╝███████║╚██████╔╝███████╗\n\
 ╚═════╝   ╚═╝   ╚═╝  ╚═╝╚══════╝       ╚═════╝ ╚══════╝ ╚═════╝ ╚══════╝\n\
                                                                         \n\
\n", 12);

    logMsg(stdout, "*** Graphics library (raylib) initialized ***\n", 10);
    logMsg(stdout, "*** CPU and memory initialized! ***\n\n", 10);

        logMsg(stdout, "\
  ###     ###     ###     ###      ###     ###     \n\
 # * # * # * # * # GOOD AFTERNOON # * # * # * # * #\n\
      ###     ###     ###     ###      ###     ### \n\n", 8);

    MSG = "*** Starting BIOS (from $%04X) ***\n\n";
    snprintf(BUFF, 256, MSG, SYS->PC);
    logMsg(stdout, BUFF, 8);

    // Raylib global init stuff
    font = LoadFontEx("FONTS/PxPlus_ToshibaSat_8x14.ttf", 14, 0, 437);
    
    currentScreen = INIT;
    InitInitScreen();
    SetTargetFPS(60);

    // Main raylib loop
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
        
    }

    // De-init
    switch (currentScreen) {
        case INIT: UnloadInitScreen(); break;
        case MAIN: UnloadMainScreen(); break;
        default: break;
    }

    // Unload global data
    UnloadFont(font);
    UnloadMusicStream(audio);
    UnloadSound(soundfx);

    CloseAudioDevice();

    CloseWindow();


    //CPU_RUNNING = !CPU_RUNNING;
    /*
    while (CPU_RUNNING) {
        MEM_PRINT(SYSMEM, 0xFD00, 64, 8);

        MEM_TO_BIN(SYSMEM, "SYS/OUT.mc", 0xFD00, 32);
        
        GRAPHICS_INIT();
        printf("test\n");
        CPU_RUNNING = false;
    }*/

    return MAIN_STATUS;
}

HBYTE INSTR_FETCH_AND_EXECUTE(INSTR** INSTR_SET, MEM* SYSMEM, CPU* SYS) {
    // Most recently obtained instruction opcode
    HBYTE OPC = 0x0;
    HBYTE ADDR_MODE = 0x0;
    // 8-bit data container
    HBYTE OP_H = 0x0;
    // 16-bit data container
    ADDRESS OP_A = 0x0;

    HBYTE RES = 0x0;

    INSTR* INSTR_OBJ;
    int (*INSTR_FX)(MEM*, CPU*, HBYTE, ADDRESS);

    // Read instruction from data at location of SYS->PC
    OPC = MEM_READ(SYSMEM, SYS->PC);

    // Retrieve instruction object from opcode array
    INSTR_OBJ = INSTR_SET[OPC];

    // Unpack addressing mode, function pointer from object
    ADDR_MODE = INSTR_OBJ->A_MODE;
    INSTR_FX = INSTR_OBJ->FUNC;

    //MSG = "CURRENT INSTRUCTION: %02X (addr mode: %d)\n";
    //snprintf(BUFF, 256, MSG, OPC, ADDR_MODE);
    //logMsg(stdout, BUFF, 12);

    // Read operand data from memory according to addressing mode
    // Call instruction function using addressing mode and operand
    switch (ADDR_MODE) {
        // Read next byte
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 11:
        case 12:
            SYS->PC++;
            OP_A = MEM_READ(SYSMEM, SYS->PC);

            RES = INSTR_FX(SYSMEM, SYS, ADDR_MODE, OP_A);
            break;

        // Read next two bytes
        case 7:
        case 8:
        case 9:
        case 10:
            SYS->PC++;
            OP_H = MEM_READ(SYSMEM, SYS->PC);

            SYS->PC++;
            OP_A = 256 * MEM_READ(SYSMEM, SYS->PC);
            OP_A += OP_H;

            RES = INSTR_FX(SYSMEM, SYS, ADDR_MODE, OP_A);
            break;

        // For implied and accumulator modes
        // Just start executing instruction
        default:
            RES = INSTR_FX(SYSMEM, SYS, 0, 0);
            break;
    }

    return RES;
}

ADDRESS CPU_ISR_HANDLER(MEM* SYSMEM, CPU* SYS) {
    //char *MSG;
    //char BUFF[256];

    // Interrupt-Service Routine handler function
    //////////////////////////////////////////////
    // When interrupt request is called,
    //      1. Pushes HIGH BYTE of return address to stack
    //      2. Pushes LOW BYTE of return address to stack
    //      3. Pushes status register to stack
    //      4. Get IRQ/NMI vector LOW BYTE from $FFFE/A
    //      5. Get IRQ/NMI vector HIGH BYTE from $FFFF/B
    //      6. Jump to vector and execute ISR

    // Load address from $FFFE/FFFF by default
    HBYTE VEC_OFFSET = 0x4;

    // Splits return address into high and low bits to push to stack
    HBYTE* RETURN_SPLIT = ADDR_SPLIT(SYS->PC);
    
    // If normal BRK
    if (SYS->BRK){
        // Default load is $FFFE/FFFF
    }
    // Load address from $FFFA/FFFB
    else if (SYS->NMI) {
        VEC_OFFSET = 0x0;
    }
    // Load address from $FFFC/FFFD
    else if (SYS->RST) {
        VEC_OFFSET = 0x2;
    }
    else if (SYS->IRQ){
        // Check if interrupt-disable is TRUE
        if (CPU_GET_STATUS(SYS, 2)) {
            return 0xFFFF;
        }

        CPU_SET_STATUS(SYS, 2, true);
    }

    // Push HIGH BIT of return address to stack
    STACK_PUSH(SYSMEM, SYS, RETURN_SPLIT[0], NULL);
    // Push LOW BIT of return address to stack
    STACK_PUSH(SYSMEM, SYS, RETURN_SPLIT[1], NULL);

    // Push STATUS REGISTER onto stack
    STACK_PUSH(SYSMEM, SYS, SYS->STATUS, NULL);

    JMP(SYSMEM, SYS, 10, 0xFFFA + VEC_OFFSET);
    
    //printf("Reset vector $%04x from LOC{$%04x}\n", SYS->PC, 0xFFFA+VEC_OFFSET);
    //MEM_PRINT(SYSMEM, 0x1F0, 64, 8);

    return SYS->PC;
}

bool CPU_SET_STATUS(CPU* SYS, HBYTE FLAG, bool VAL) {
    //////////////////
    // 76543210
    // NV1BDIZC
    //////////////////

    // Current value of flag to be changed
    bool PREV = (SYS->STATUS << FLAG) & (HBYTE)1;

    // Set bit to desired value
    if (VAL == true) {
        SYS->STATUS = SYS->STATUS | ((HBYTE)1 << FLAG);
    }
    else if (VAL == false) {
        SYS->STATUS = SYS->STATUS & ~((HBYTE)1 << FLAG);
    }

    return PREV;
}

bool CPU_GET_STATUS(CPU* SYS, HBYTE FLAG) {
    //////////////////
    // 76543210
    // NV1BDIZC
    //////////////////
    return ((SYS->STATUS << FLAG) & (HBYTE)1);
}

int INSTR_DECODE(INSTR** INSTR_SET, MEM* SYSMEM, CPU* SYS, HBYTE* OPCODE, ADDRESS* DATA) {
    // Get instruction object from hex value OPCODE
    // Object contains function pointer for instruction, proper addressing mode
    INSTR* DECODE = INSTR_SET[*OPCODE];
    //HBYTE ADDR_MODE = DECODE->A_MODE;

    return (*DECODE->FUNC)(SYSMEM, SYS, 0, 0);
}

int ADC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // ADd with Carry

    // Affects flags:
    // N, V, Z, C
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);

        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
                    
            break;
    }

    return (OP_H + OP_A);
}
int AND(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // bitwise AND with accumulator

    // Affects flags:
    // N, Z

    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H + OP_A);
}
int ASL(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Arithmetic Shift Left

    // Affects flags:
    // N, Z, C

    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Accumulator
        case 1:

            OP_H = 0;
            OP_A = 0;

        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int BCC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on Carry Clear

    // Affects flags:
    // NONE

    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return OP_H;
}
int BCS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on Carry Set

    // Affects flags:
    // NONE
    
    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H);
}
int BEQ(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on EQual

    // Affects flags:
    // NONE
    
    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H);
}
int BIT(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // test BITs

    // Affects flags:
    // N, V, Z

    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int BMI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on MInus

    // Affects flags:
    // NONE
    
    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H);
}
int BNE(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on Not Equal

    // Affects flags:
    // NONE
    
    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H);
}
int BPL(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on PLus

    // Affects flags:
    // NONE
    
    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H);
}
int BRK(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // BReaK

    // Affects flags:
    // B
    
    switch (ADDR_MODE) {
        // Implied
        case 0:
            // Behaves like NMI
            SYS->BRK = true;
            // Set Break flag, even though its value is ignored
            CPU_SET_STATUS(SYS, 4, 1);
            // Call ISR
            CPU_ISR_HANDLER(SYSMEM, SYS);
            break;
    }

    return 0;
}
int BVC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on oVerflow Clear

    // Affects flags:
    // NONE
    
    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H);
}
int BVS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Branch on oVerflow Set

    // Affects flags:
    // NONE
    
    HBYTE OP_H = 0;

    switch (ADDR_MODE) {
        // Relative
        case 6:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H);
}
int CLC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // CLear Carry flag

    // Affects flags:
    // C

    switch (ADDR_MODE) {
        // Implied
        case 0:
            // Set carry flag to 0
            CPU_SET_STATUS(SYS, 0, false);
            break;
    }

    return 0;
}
int CLD(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // CLear Decimal mode

    // Affects flags:
    // D

    switch (ADDR_MODE) {
        // Implied
        case 0:
            // Set decimal flag to 0
            CPU_SET_STATUS(SYS, 3, 0);
            break;
    }


    return 0;
}
int CLI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // CLear Interrupt disable bit

    // Affects flags:
    // I

    switch (ADDR_MODE) {
        // Implied 
        case 0:
            // Set interrupt disable flag to 0
            CPU_SET_STATUS(SYS, 2, 0);
            break;
    }

    return 0;
}
int CLV(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // CLear oVerflow flag

    // Affects flags:
    // O

    switch (ADDR_MODE) {
        // Implied
        case 0:
            // Set overflow flag to 0
            CPU_SET_STATUS(SYS, 6, 0);
            break;
    }

    return 0;
}
int CMP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // CoMPare accumulator

    // Affects flags:
    // N, Z, C

    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H + OP_A);
}
int CPX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // ComPare memory and index X

    // Affects flags:
    // N, Z, C

    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int CPY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // ComPare memory and index Y

    // Affects flags:
    // N, Z, C
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
                    
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int DEC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    //DECrement memory by one

    // Affects flags:
    // N, Z
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
            break;
    }
    return (OP_H + OP_A);
}
int DEX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // DEcrement index X by one

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }
    return 0;
}
int DEY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // DEcrement index Y by one

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }
    return 0;
}
int EOR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Exclusive-OR memory with accumulator

    // Affects flags:
    // N, Z
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
                    
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }
    return (OP_H + OP_A);
}
int INC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // INCrement memory by one

    // Affects flags:
    // N, Z
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
                    
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int INX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // INcrement index X by one

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int INY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // INcrement index Y by one

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int JMP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // JuMP to new location

    // Affects flags:
    // None

    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Absolute
        case 7:
            // Sets Program Counter to address specified by OP_A
            OP_A = OPERAND;

            SYS->PC = OP_A;
            
            break;

        // Indirect
        case 10:
            // Sets Program Counter to value stored in memory at location OP_A
            OP_A = OPERAND;
            HBYTE JUMP_LOC[2];

            // $LLHH      Data 
            //  * **      Address
            //          *: $(OP_A)
            //         **: $(OP_A + 1)
            JUMP_LOC[1] = MEM_READ(SYSMEM, OP_A);
            JUMP_LOC[0] = MEM_READ(SYSMEM, OP_A + 1);
            // Jump_Loc:
            // Index: 0  1
            //       HH LL

            ADDRESS JUMP_DEST = ADDR_CONCAT(JUMP_LOC);
            
            SYS->PC = JUMP_DEST;
            
            break;
    }

    return (OP_A);
}
int JSR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Jump to new location Saving Return address

    // Affects flags:
    // None

    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Absolute
        case 7:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_A);
}
int LDA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // LoaD accumulator with memory

    // Affects flags:
    // N, Z
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
                    
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H + OP_A);
}
int LDX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // LoaD index X with memory

    // Affects flags:
    // N, Z
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, Y
        case 5:
            OP_H = (HBYTE) (OPERAND);
                    
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int LDY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // LoaD index Y with memory

    // Affects flags:
    // N, Z
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
                    
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int LSR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // shift LSB right (memory or accumulator)

    // Affects flags:
    // N, Z, C
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Accumulator
        case 1:

            OP_H = 0;
            OP_A = 0;

        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int NOP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // No OPeration

    // Affects flags:
    // None

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return (0);
}
int ORA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // OR memory with accumulator

    // Affects flags:
    // N, Z
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
                    
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H + OP_A);
}
int PHA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // PusH accumulator on stack

    // Affects flags:
    // None


    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int PHP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // PusH Processor status on stack

    // Affects flags:
    // None


    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int PLA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // PuLl Accumulator from stack

    // Affects flags:
    // N, Z


    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int PLP(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // PuLl Processor status from stack

    // Affects flags:
    // From stack (?)


    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int ROL(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Rotate One bit Left

    // Affects flags:
    // N, Z, C
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Accumulator
        case 1:

            OP_H = 0;
            OP_A = 0;
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int ROR(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Rotate One bit Right

    // Affects flags:
    // N, Z, C
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Accumulator
        case 1:

            OP_H = 0;
            OP_A = 0;

        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int RTI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // ReTurn from Interrupt

    // Affects flags:
    // From stack (?)


    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int RTS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // ReTurn from Subroutine

    // Affects flags:
    // None
    

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int SBC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // SuBtraCt memory from accumulator with borrow

    // Affects flags:
    // N, Z, C, V
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Immediate
        case 2:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
                    
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H + OP_A);
}
int SEC(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // SEt Carry flag

    // Affects flags:
    // C

    switch (ADDR_MODE) {
        // Implied
        case 0:
            // Set carry flag to 1
            CPU_SET_STATUS(SYS, 0, 1);
            break;
    }

    return 0;
}
int SED(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // SEt Decimal flag

    // Affects flags:
    // D

    switch (ADDR_MODE) {
        // Implied
        case 0:
            // Set decimal flag to 1
            CPU_SET_STATUS(SYS, 3, 1);
            break;
    }

    return 0;
}
int SEI(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // SEt Interrupt disable status

    // Affects flags:
    // I

    switch (ADDR_MODE) {
        // Implied
        case 0:
            // Set interrupt disable flag to 1
            CPU_SET_STATUS(SYS, 2, 1);
            break;
    }

    return 0;
}
int STA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // STore Accumulator in memory

    // Affects flags:
    // None
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
                    
        // Absolute
        case 7:
            OP_A = OPERAND;
            
        // Absolute, X
        case 8:
            OP_A = OPERAND;
            
        // Absolute, Y
        case 9:
            OP_A = OPERAND;
            
        // Indirect, X
        case 11:
            OP_H = (HBYTE) (OPERAND);
            
        // Indirect, Y
        case 12:
            OP_H = (HBYTE) (OPERAND);
            
            break;
    }

    return (OP_H + OP_A);
}
int STX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // STore index X in memory

    // Affects flags:
    // None
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, Y
        case 5:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int STY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // STore index Y in memory

    // Affects flags:
    // None
    
    HBYTE OP_H = 0;
    ADDRESS OP_A = 0;

    switch (ADDR_MODE) {
        // Zero Page
        case 3:
            OP_H = (HBYTE) (OPERAND);
            
        // Zero Page, X
        case 4:
            OP_H = (HBYTE) (OPERAND);
            
        // Absolute
        case 7:
            OP_A = OPERAND;
            
            break;
    }

    return (OP_H + OP_A);
}
int TAX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Transfer Accumulator to index X

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int TAY(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Transfer Accumulator to index Y

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int TSX(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Transfer Stack pointer to index X

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int TXA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Transfer index X to Accumulator

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int TXS(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Transfer index X to Stack register

    // Affects flags:
    // None

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}
int TYA(MEM* SYSMEM, CPU* SYS, HBYTE ADDR_MODE, ADDRESS OPERAND) {
    // Transfer index Y to Accumulator

    // Affects flags:
    // N, Z

    switch (ADDR_MODE) {
        // Implied
        case 0:

            break;
    }

    return 0;
}