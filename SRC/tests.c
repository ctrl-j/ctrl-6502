#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "mem.h"
#include "tests.h"
#include "cpu.h"

/*int main(int argc, char** argv) {
    bool status;
    
    status = TEST_CPU_MAIN();

    if (status == true) {
        printf("All tests passed 8==D\n");
    }

    return EXIT_SUCCESS;
}
*/

bool TEST_STACK_PUSH_POP(MEM* SYSMEM) {


    return true;
}

bool TEST_CPU_MAIN() {
    
    return true;
}

bool TEST_ADDR_SPLIT() {
    ADDRESS tas[5] = {0x0101, 0x4B3D, 0x123A, 0x82BA, 0x2ABC};
    int i;
    HBYTE* res;

    for (i = 0; i < 5; i++) {
        res = ADDR_SPLIT(tas[i]);
        printf("Page: %d, Index: %d\n", res[0], res[1]);
    }
    return true;
}

bool TEST_MEM_READ() {

    return true;
}

bool TEST_MEM_WRITE() {
    MEM* SYSMEM = malloc(sizeof(MEM));
    HBYTE DATA[20] = {0};

    uint16_t addr_counter = 0x0;

    for (addr_counter = 0; addr_counter < 0x10; addr_counter++) {
        MEM_WRITE(SYSMEM, addr_counter, DATA[addr_counter]);
        
    }

    return true;
}