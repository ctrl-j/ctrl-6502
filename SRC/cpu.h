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

// Instead of having separate X, Y registers:
//      For accuracy, have CURR_IND_REG and PREV_IND_REG,
//      as well as some flag keeping track of which is "X" and which is "Y".
//      That way you just swap the addresses of the two and update the ID flags,
//      instead of having to swap data

#ifndef __CPU_H__
#define __CPU_H__

#endif