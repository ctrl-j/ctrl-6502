#include <stdint.h>
#include <stdbool.h>

typedef uint8_t BYTE;
typedef uint16_t WORD;

typedef struct _CPU {

    WORD ADDRESS;
    BYTE DATA;

    WORD PC;
    
    BYTE AC;
    BYTE SP;
    BYTE X;
    BYTE Y;

    bool NEGATIVE;
    bool OVERFLOW;

    bool BREAK;
    bool DECIMAL;
    bool INTERRUPT_DISABLE;
    bool ZERO;
    bool CARRY;

    //////// Hardware pins ////////

    // Read/write: read = 1 , write = 0
    bool RW;

    bool IRQ;
    bool NMI;
    bool RST;

} CPU;

