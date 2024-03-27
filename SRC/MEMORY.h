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

#ifndef __MEMORY_H__
#define __MEMORY_H__

// Flashes system RAM from binary source
//      -MEMORY (MEM*)
//          structure containing memory HBYTE array
//      -SRC (char*)
//          filename of binary file containing machine code
//      -SRC_START (ADDRESS)
//          Line of SRC to start loading code from. Must be < SRC_WIDTH
//      -SRC_WIDTH (ADDRESS)
//          0: Loads entire SRC file
//          1 - (MAX ROM WIDTH): Loads as many bytes from SRC as it can,
//                               up to SRC_WIDTH
//          FFFF: flashes entire 256 pages including system mem and BIOS
//                Loads default BIOS and system mem values
//                Ignores other options except MEMORY, SRC
//      -MEM_START (ADDRESS)
//          Desired start location of write to RAM
//      -PRESERVE (bool)
//          0: write 0's to rest of RAM
//          1: preserve RAM other than flashed memory
void MEM_LOAD(MEMORY* MEMORY, char* SRC,
                   WORD SRC_START, WORD SRC_WIDTH,
                   WORD MEM_START, bool PRESERVE);

// Exports portion of system memory to [DST].le binary file
// Loads {$MEM_START - $(MEM_START + MEM_WIDTH)}
// If MEM_WIDTH = 0, contents are copied until end of memory
void MEM_SAVE(MEMORY* MEMORY, char* DST,
                WORD MEM_START, WORD MEM_WIDTH);                  

#endif