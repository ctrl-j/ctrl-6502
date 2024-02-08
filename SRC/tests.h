/*
    Copyright (C) 2024  Joshua Jackson
    [Full GPL-3.0 license can be found in LICENSE, in the root folder]
*/

#ifndef __TESTS_H__
#define __TESTS_H__

#include "mem.h"

bool TEST_ADDR_SPLIT();

bool TEST_MEM_WRITE();
bool TEST_MEM_READ();

bool TEST_CPU_MAIN();

bool TEST_STACK_PUSH_POP(MEM* SYSMEM);

#endif