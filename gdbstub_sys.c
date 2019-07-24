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

#include "gdbstub.h"
#include <string.h>

static struct dbg_state    dbg_state;

void dbg_sys_load(FILE *fp)
{
	memset(&dbg_state, 0, sizeof(dbg_state));
	char buff[256];
	const char *regs = "--- begin regs ---";
	const char *mem = "--- begin memory ---";
	while (fgets(buff, sizeof(buff), fp)) {
		if (!strncmp(buff, regs, strlen(regs))) {
			fscanf(fp, "%x", &dbg_state.registers[0]);  // PC
			fscanf(fp, "%x", &dbg_state.registers[36]); // SAR
			fscanf(fp, "%x", &dbg_state.registers[37]); // LITBASE
			fscanf(fp, "%x", &dbg_state.registers[40]); // SR176
			fscanf(fp, "%x", &dbg_state.registers[42]); // PS
			for (int i=0; i<16; i++) {
				fscanf(fp, "%x", &dbg_state.registers[97 + i]); // A[0]..A[15]
			}
		} else if (!strncmp(buff, mem, strlen(mem))) {
			for (int i=0; i<80 * 1024; i++ ) {
				int t;
				fscanf(fp, "%02x", &t);
				dbg_state.mem[i] = t;
			}
		}
	}
	dbg_state.registers[0] = 0x4010569c;
}

/*
 * Write one character to the debugging stream.
 */
int dbg_sys_putchar(int ch)
{
	int ret = putchar(ch);
	fflush(stdout);
	return ret;
}

/*
 * Read one character from the debugging stream.
 */
int dbg_sys_getc(void)
{
	int ret = getchar() & 0xff;
	return ret;
}

/*
 * Read one byte from memory.
 */
int dbg_sys_mem_readb(address addr, char *val)
{
	if (addr < 0x3FFEC000 || addr >= 0x40000000) {
		return -1;
	}
	*val = dbg_state.mem[addr - 0x3FFEC000];
	return 0;
}

/*
 * Write one byte to memory.
 */
int dbg_sys_mem_writeb(address addr, char val)
{
	if (addr < 0x3FFEC000 || addr >= 0x40000000) {
		return -1;
	}
	dbg_state.mem[addr - 0x3FFEC000] = val;
	return 0;
}

/*
 * Continue program execution.
 */
int dbg_sys_continue(void)
{
	return -1;
}

/*
 * Single step the next instruction.
 */
int dbg_sys_step(void)
{
	return -1;
}


extern int dbg_main(struct dbg_state *state);

int main(int argc, char **argv)
{
	FILE *f = fopen("crash.log", "r");
	dbg_sys_load(f);
	fclose(f);
	dbg_main(&dbg_state);
}

