/*
 * Copyright (C) 2016  Matt Borgerson
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdint.h>

typedef uint32_t address;
typedef uint32_t reg;
#define DBG_NUM_REGISTERS 112

typedef struct mem_region {
	uint32_t           base;
	uint32_t           size;
	uint8_t           *data;
	struct mem_region *next;
} mem_region;

struct dbg_state {
	uint32_t registers[DBG_NUM_REGISTERS]; // Most are undefined
	mem_region *mem;
};

void dbg_sys_load(const char *fname);     /* Parse dump into dbg_state */
void dbg_sys_load_elf(const char *fname); /* ELF binary being debugged */

